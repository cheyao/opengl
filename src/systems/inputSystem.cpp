/**
 * @file inputSystem.cpp
 * @brief Implementation of the InputSystem, responsible for handling 
 *        user input events such as keypresses and mouse interactions.
 *
 * This system processes player input each frame and transforms events into
 * game actions (e.g. block placement, block destruction, or UI interactions).
 * It also renders a "block destruction overlay" if the player is in the
 * process of breaking a block.
 */

#include "systems/inputSystem.hpp"

#include "components.hpp"
#include "components/playerInventory.hpp"
#include "game.hpp"
#include "managers/eventManager.hpp"
#include "managers/systemManager.hpp"
#include "misc/sparse_set_view.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"
#include "systems/physicsSystem.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <cmath>

/**
 * @class InputSystem
 * @brief Handles keyboard and mouse input, deferring logic to user-defined
 *        functions or internal helpers.
 *
 * - Keyboard inputs are passed to an `input.mFunction` callback stored in
 *   each entity's Components::input struct.
 * - Mouse inputs handle block placement (right-click) or block destruction (left-hold).
 * - Interaction with the UI system is also considered, ignoring input if a UI overlay is open.
 */

InputSystem::InputSystem() noexcept
   : mGame(Game::getInstance()),
     mDestruction({})
{
}

/**
 * @brief Primary update function for InputSystem, called once per frame.
 * @param scene  Pointer to the current scene.
 * @param delta  Time-step since the last frame in seconds.
 */
void InputSystem::update(Scene* scene, float delta)
{
   // If a UI overlay is open (e.g. an inventory screen), ignore other inputs.
   if (!mGame || !scene) {
      // Defensive check; if references are invalid, skip processing.
      return;
   }

   if (!mGame->getSystemManager()->getUISystem()->empty()) {
      return;
   }

   // Retrieve the latest keyboard state from the Game object.
   const auto keystate = mGame->getKeystate();

   // For each entity that has an input component, call its custom input function.
   for (auto&& [entity, input] : scene->view<Components::input>().each()) {
      if (!input.mFunction) {
         // If there's no function, continue to the next entity
         continue;
      }
      // Execute the entity's input function with the current scene, entity ID, keystate, and delta.
      input.mFunction(scene, entity, keystate, delta);
   }

   // Handle mouse interactions each frame (block placement, destruction overlay, etc.).
   updateMouse(scene, delta);
}

/**
 * @brief Handles all mouse-related logic, including block destruction and placement.
 * @param scene  Pointer to the current scene.
 * @param delta  Time-step since the last frame in seconds (not currently used here).
 */
void InputSystem::updateMouse(Scene* scene, float /*delta*/)
{
   if (!scene) return;

   // We reset the destruction overlay by default to not render,
   // and enable it only if a block is currently being destroyed.
   mDestruction.render = false;

   // Obtain the mouse position in screen coordinates (pixels).
   float mouseX = 0.0f;
   float mouseY = 0.0f;
   SDL_GetMouseState(&mouseX, &mouseY);

   // Convert Y to "OpenGL coords" by flipping top-to-bottom.
   const auto windowSize = mGame->getSystemManager()->getDemensions();
   mouseY = windowSize.y() - mouseY;

   // Get the player's position to adjust for camera offset.
   const auto& playerPosComp = scene->get<Components::position>(mGame->getPlayerID());
   const auto& playerPos     = playerPosComp.mPosition;

   // Translate from screen-space mouse coords to "world" coords around the player.
   const float realX = mouseX + playerPos.x() - (windowSize.x() * 0.5f);
   const float realY = mouseY + playerPos.y() - (windowSize.y() * 0.5f);

   // Convert those coords to block coordinates. 
   // The minus-one offsets handle negative division properly.
   const Eigen::Vector2i blockPos(
      static_cast<int>(realX) / Components::block::BLOCK_SIZE - (realX < 0 ? 1 : 0),
      static_cast<int>(realY) / Components::block::BLOCK_SIZE - (realY < 0 ? 1 : 0)
   );

   // Check for right-click signals to place a block in that location.
   if (scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL)) {
      scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;
      tryPlace(scene, blockPos);
   }

   // Track how long left-click has been held down in the same block, to handle destruction.
   static std::int64_t sLastHoldTime = SDL_GetTicks64();

   /**
    * @brief Lambda to handle the left-click logic:
    *        - If the block changed since last update, reset hold time.
    *        - If the user is still holding left-click, check how "destroyed" the block is.
    *        - If fully destroyed, remove the block from the scene and drop items.
    */
   auto handleLeftClick = [&]() {
      // If the block the user is "aiming" at has changed, reset the hold timer.
      if (mDestruction.pos != blockPos) {
         sLastHoldTime = SDL_GetTicks64();
      }
      // Update the destruction's position reference so we can do partial overlay
      // if the user continues holding left-click in the same block.
      mDestruction.pos = blockPos;

      // If the scene no longer registers a left-hold signal, do nothing.
      if (!scene->getSignal(EventManager::LEFT_HOLD_SIGNAL)) {
         return;
      }

      // Calculate how many "units of press" have passed, in increments of 50ms.
      float pressLength = static_cast<float>(SDL_GetTicks64() - sLastHoldTime) / 50.0f;

      // Iterate over all blocks in the scene, comparing position to the block of interest.
      for (auto&& [entity, block] : scene->view<Components::block>().each()) {
         if (block.mPosition != blockPos) {
            continue;
         }

         // Retrieve break time for the block type from a register or fallback to 1.0f.
         const auto breakTimeIt = registers::BREAK_TIMES.find(block.mType);
         if (breakTimeIt == registers::BREAK_TIMES.end()) {
            // If not found, skip or assume a default break time.
            continue;
         }
         const float blockBreakTime = breakTimeIt->second;

         // If the user hasn't pressed long enough, show the partial "destroy stage".
         if (pressLength < blockBreakTime) {
            mDestruction.render = true;

            // The "stage" is from 0..10, so multiply ratio by 10.
            // Stage must be clamped to [0..9] if the ratio is in [0..1).
            const int stage = static_cast<int>((pressLength / blockBreakTime) * 10.0f);
            const std::string texPath = "blocks/destroy_stage_" + std::to_string(stage) + ".png";
            mDestruction.texture = mGame->getSystemManager()->getTexture(texPath, /*filtered=*/true);

            break;
         }

         // If fully broken, drop loot. Then remove the block entity from the scene.
         std::vector<std::pair<float, Components::Item>> defaultLoot = {
            {1.0f, block.mType}
         };
         const auto lootTableIt = registers::LOOT_TABLES.find(block.mType);
         const auto& lootTable =
            (lootTableIt != registers::LOOT_TABLES.end()) ? lootTableIt->second : defaultLoot;

         for (auto&& [chance, itemType] : lootTable) {
            // Roll a float between 0..1 to check if we drop each item
            float roll = SDL_randf(); 
            if (roll < chance) {
               // Create a new item entity
               auto newItem = scene->newEntity();
               const auto blockCenterPos = block.mPosition.cast<float>() + Eigen::Vector2f(0.40f, 0.40f);
               scene->emplace<Components::position>(newItem, blockCenterPos * Components::block::BLOCK_SIZE);
               scene->emplace<Components::item>(newItem, itemType);
               const auto texPathForLoot = registers::TEXTURES.at(itemType);
               scene->emplace<Components::texture>(newItem, mGame->getSystemManager()->getTexture(texPathForLoot), 0.3f);
            }
         }

         // Remove the block entity from the scene
         scene->erase(entity);

         // Clear the left-hold signal so it doesn't continue destroying new blocks
         scene->getSignal(EventManager::LEFT_HOLD_SIGNAL) = 0;

         // Mark the physics system as dirty if it depends on tile collisions
         scene->getSignal(PhysicsSystem::PHYSICS_DIRTY_SIGNAL) = true;

         // Done with destruction handling for this block.
         break;
      }
   };

   // Actually run the left-click logic.
   handleLeftClick();
}

/**
 * @brief Render function to draw the "destruction overlay" block.
 * @param scene  Pointer to the current scene (unused except for offset calculations).
 */
void InputSystem::draw(Scene* scene)
{
   if (!mDestruction.render) {
      return;
   }

   // Setup blending for the destruction overlay
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);

   auto* systemManager = mGame->getSystemManager();
   if (!systemManager) return;

   // Retrieve camera offset from the player's position
   const auto& playerPos = scene->get<Components::position>(mGame->getPlayerID()).mPosition;
   const Eigen::Vector2f cameraOffset = -playerPos + systemManager->getDemensions() * 0.5f;

   // Use a dedicated block shader
   Shader* shader = systemManager->getShader("block.vert", "block.frag");
   if (!shader) {
      return;
   }

   // Activate the shader and set uniform parameters
   shader->activate();

   shader->set("size"_u,
      static_cast<float>(Components::block::BLOCK_SIZE),
      static_cast<float>(Components::block::BLOCK_SIZE));
   shader->set("texture_diffuse"_u, 0);
   shader->set("offset"_u, cameraOffset);
   shader->set("position"_u, mDestruction.pos);
   shader->set("scale"_u, 1.0f);

   // Bind the destruction stage texture
   if (mDestruction.texture) {
      mDestruction.texture->activate(0);
   }
   else {
      // Ideally handle missing texture gracefully, e.g., skipping
      return;
   }

   // Draw the block mesh (the same used for standard block rendering).
   auto* uiSys = systemManager->getUISystem();
   if (uiSys) {
      Mesh* mesh = uiSys->getMesh();
      if (mesh) {
         mesh->draw(shader);
      }
   }
}

/**
 * @brief Attempts to place a block at the specified position (in block coords).
 * @param scene Pointer to the current Scene.
 * @param pos   The block coordinate position where the user wants to place a block.
 */
void InputSystem::tryPlace(Scene* scene, const Eigen::Vector2i& pos)
{
   if (!scene) return;

   using namespace Components;

   // Access the player's inventory
   auto& invComp = scene->get<inventory>(mGame->getPlayerID());
   auto* invPtr  = static_cast<PlayerInventory*>(invComp.mInventory);
   if (!invPtr) {
      return;
   }

   // Check if a block already occupies this position
   for (auto&& [entity, block] : scene->view<Components::block>().each()) {
      if (block.mPosition == pos) {
         // There's already a block, so do nothing.
         return;
      }
   }

   // Basic bounding box for the block to place (stone size is used as a reference).
   const Texture* stoneTex = mGame->getSystemManager()->getTexture("blocks/stone.png", true);
   Eigen::Vector2f minB = pos.cast<float>() * block::BLOCK_SIZE + Eigen::Vector2f(5.0f, 5.0f);
   Eigen::Vector2f sizeVec = (stoneTex) ? stoneTex->getSize() * 0.9f : Eigen::Vector2f(16.0f, 16.0f);
   Eigen::Vector2f maxB = minB + sizeVec;

   // Check for collision with any entity that has a position+collision component
   for (auto&& entity : scene->view<Components::position, Components::collision>()) {
      auto& p    = scene->get<position>(entity);
      auto& coll = scene->get<collision>(entity);

      // Build bounding box for existing entity
      Eigen::Vector2f minA = p.mPosition + coll.mOffset;
      Eigen::Vector2f maxA = minA + coll.mSize;

      // If bounding boxes overlap, can't place the block
      bool notIntersecting = (maxA.x() <= minB.x()) || // A on left side
                             (maxA.y() <= minB.y()) || // A below
                             (maxB.x() <= minA.x()) || // B on left side
                             (maxB.y() <= minA.y());   // B below
      if (!notIntersecting) {
         // They do intersect
         return;
      }
   }

   // If we made it this far, call the player's inventory "tryPlace" logic to
   // place the block in the world.
   invPtr->tryPlace(scene, pos);
}


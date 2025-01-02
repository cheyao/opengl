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

InputSystem::InputSystem() noexcept : mGame(Game::getInstance()) {}

void InputSystem::update(Scene* scene, const float delta) {
	if (!mGame->getSystemManager()->getUISystem()->empty()) {
		return;
	}

	const auto keystate = mGame->getKeystate();

	for (const auto& [entity, input] : scene->view<Components::input>().each()) {
		SDL_assert(input.mFunction != nullptr);

		input.mFunction(scene, entity, keystate, delta);
	}

	updateMouse(scene, delta);
}

void InputSystem::updateMouse(Scene* scene, const float) {
	mDestruction.render = false;

	// From Topright
	float mouseZ = 0, mouseY = 0;
	SDL_GetMouseState(&mouseZ, &mouseY);

	// Convert Y to opengl cords
	const auto windowSize = mGame->getSystemManager()->getDemensions();
	mouseY = windowSize.y() - mouseY;

	const auto playerPos = scene->get<Components::position>(mGame->getPlayerID()).mPosition;
	const int realX = mouseZ + playerPos.x() - windowSize.x() / 2;
	const int realY = mouseY + playerPos.y() - windowSize.y() / 2;
	const Eigen::Vector2i blockPos{realX / Components::block::BLOCK_SIZE - (realX < 0),
				       realY / Components::block::BLOCK_SIZE - (realY < 0)};

	if (scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL)) {
		scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;

		tryPlace(scene, blockPos.template cast<int>());
	}

	static std::int64_t mLastHold = SDL_GetTicks();
	const auto& handleLeftClick = [&]() {
		if (mDestruction.pos != blockPos) {
			mLastHold = SDL_GetTicks();
		}

		mDestruction.pos = blockPos;

		if (!scene->getSignal(EventManager::LEFT_HOLD_SIGNAL)) {
			return;
		}

		const auto pressLength = (SDL_GetTicks() - mLastHold) / 50.0f;
		for (const auto& [entity, block] : scene->view<Components::block>().each()) {
			if (block.mPosition != blockPos) {
				continue;
			}

			// Not enough time passed since press
			if (pressLength < registers::BREAK_TIMES.at(block.mType)) {
				mDestruction.render = true;

				const int stage = (pressLength / registers::BREAK_TIMES.at(block.mType)) * 10;
				mDestruction.texture = mGame->getSystemManager()->getTexture(
					"blocks/destroy_stage_" + std::to_string(stage) + ".png", true);

				break;
			}

			const std::vector<std::pair<float, Components::Item>> defaultLoot = {{1.0f, block.mType}};
			const std::vector<std::pair<float, Components::Item>>& loot =
				registers::LOOT_TABLES.contains(block.mType) ? registers::LOOT_TABLES.at(block.mType)
									     : defaultLoot;

			for (const auto& [chance, type] : loot) {
				const float roll = SDL_randf();
				if (roll >= chance) {
					continue;
				}

				const auto item = scene->newEntity();
				scene->emplace<Components::position>(
					item, (scene->get<Components::block>(entity).mPosition.template cast<float>() +
					       Eigen::Vector2f(0.40f, 0.40f)) *
						      Components::block::BLOCK_SIZE);
				scene->emplace<Components::item>(item, type);
				scene->emplace<Components::texture>(
					item, mGame->getSystemManager()->getTexture(registers::TEXTURES.at(type)),
					0.3f);
			}

			scene->erase(entity);
			scene->getSignal(EventManager::LEFT_HOLD_SIGNAL) = 0;

			// Unused for the moment
			scene->getSignal(PhysicsSystem::PHYSICS_DIRTY_SIGNAL) = true;

			break;
		}
	};

	handleLeftClick();
}

void InputSystem::draw(class Scene* scene) {
	if (!mDestruction.render) {
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);

	const auto systemManager = mGame->getSystemManager();
	const auto playerPos = scene->get<Components::position>(mGame->getPlayerID()).mPosition;
	const Eigen::Vector2f cameraOffset = -playerPos + systemManager->getDemensions() / 2;

	Shader* shader = systemManager->getShader("block.vert", "block.frag");

	shader->activate();
	shader->set("size"_u, (float)Components::block::BLOCK_SIZE, (float)Components::block::BLOCK_SIZE);
	shader->set("texture_diffuse"_u, 0);
	shader->set("offset"_u, cameraOffset);
	shader->set("position"_u, mDestruction.pos);
	shader->set("scale"_u, 1.0f);

	mDestruction.texture->activate(0);
	mGame->getSystemManager()->getUISystem()->getMesh()->draw(shader);
}

void InputSystem::tryPlace(class Scene* scene, const Eigen::Vector2i& pos) {
	using namespace Components;

	auto* inv = static_cast<PlayerInventory*>(scene->get<Components::inventory>(mGame->getPlayerID()).mInventory);
	for (const auto& block : scene->view<Components::block>()) {
		if (scene->get<Components::block>(block).mPosition == pos) {
			return;
		}
	}

	const Eigen::Vector2f minB = pos.template cast<float>() * block::BLOCK_SIZE + Eigen::Vector2f(5, 5);
	const Texture* stone = mGame->getSystemManager()->getTexture("blocks/stone.png", true);
	const Eigen::Vector2f maxB = minB + stone->getSize() * 0.9;

	for (const auto& entity : scene->view<Components::position, Components::collision>()) {
		const Eigen::Vector2f minA =
			scene->get<position>(entity).mPosition + scene->get<collision>(entity).mOffset;
		const Eigen::Vector2f maxA = minA + scene->get<collision>(entity).mSize;

		// If one of these four are true, it means the cubes are not intersecting
		const bool notIntercecting = maxA.x() <= minB.x()     // Amax to the left of Bmin
					     || maxA.y() <= minB.y()  // Amax to the bottom of Bmin
					     || maxB.x() <= minA.x()  // Bmax to the left of Amax
					     || maxB.y() <= minA.y(); // Bmax to the bottom of Amin

		// So return the inverse of not intersecting
		if (!notIntercecting) {
			return;
		}
	}

	inv->tryPlace(scene, pos);
}

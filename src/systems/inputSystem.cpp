#include "systems/inputSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/systemManager.hpp"
#include "misc/sparse_set_view.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>

InputSystem::InputSystem(Game* game) : mGame(game), mPressedX(0), mPressedY(0), mPressLength(0) {
	constexpr const static float vertices[] = {
		0.0f, 0.0f, 0.0f, // TL
		0.0f, 1.0f, 0.0f, // BR
		1.0f, 0.0f, 0.0f, // TR
		1.0f, 1.0f, 0.0f  // BL
	};

	constexpr const static float texturePos[] = {
		0.0f, 1.0f, // TR
		0.0f, 0.0f, // BR
		1.0f, 1.0f, // TL
		1.0f, 0.0f  // BL
	};

	const static GLuint indices[] = {2, 1, 0,  // a
					 1, 2, 3}; // b

	mMesh = std::unique_ptr<Mesh>(new Mesh(vertices, {}, texturePos, indices, {}));
}

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

void InputSystem::updateMouse(Scene* scene, const float delta) {
	const static std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES = {
		{Components::Item::AIR, 0},
		{Components::Item::GRASS_BLOCK, 20},
		{Components::Item::STONE, 80},
	};

	mDestruction.render = false;

	// From Topright
	float mouseZ = 0, mouseY = 0;
	const SDL_MouseButtonFlags flags = SDL_GetMouseState(&mouseZ, &mouseY);

	// Convert Y to opengl cords
	const auto windowSize = mGame->getSystemManager()->getDemensions();
	mouseY = windowSize.y() - mouseY;

	const bool leftClick = flags & SDL_BUTTON_LMASK;

	if (!leftClick) {
		if (mPressLength > 0 && mPressLength < LONG_PRESS_ACTIVATION_TIME) {
			// Short click
		}

		mPressLength = 0;

		return;
	}

	const auto playerPos = scene->get<Components::position>(mGame->getPlayerID()).mPosition;
	const int realX = mouseZ + playerPos.x() - windowSize.x() / 2;
	const int realY = mouseY + playerPos.y() - windowSize.y() / 2;
	const Eigen::Vector2i blockPos{realX / Components::block::BLOCK_SIZE - (realX < 0),
				       realY / Components::block::BLOCK_SIZE - (realY < 0)};

	if (mDestruction.pos != blockPos) {
		mPressLength = 0;
	}

	mDestruction.pos = blockPos;

	mPressLength += delta;

	if (mPressLength < LONG_PRESS_ACTIVATION_TIME) {
		return;
	}

	for (const auto& [entity, block, texture] : scene->view<Components::block, Components::texture>().each()) {
		if (block.mPosition == blockPos) {
			if ((mPressLength * 20) >= BREAK_TIMES.at(block.mType)) {
				scene->erase(entity);

				mPressLength = 0;

				break;
			}

			mDestruction.render = true;

			const int stage = ((mPressLength * 20) / BREAK_TIMES.at(block.mType)) * 10;
			mDestruction.texture = mGame->getSystemManager()->getTexture(
				"blocks/destroy_stage_" + std::to_string(stage) + ".png", true);

			break;
		}
	}
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

	mDestruction.texture->activate(0);
	mMesh->draw(shader);
}

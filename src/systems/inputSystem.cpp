#include "systems/inputSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "misc/sparse_set_view.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

InputSystem::InputSystem(Game* game) : mGame(game), mPressedX(0), mPressedY(0), mPressLength(0) {}

void InputSystem::update(Scene* scene, const float delta) {
	const auto keystate = mGame->getKeystate();

	for (const auto& [entity, input] : scene->view<Components::input>().each()) {
		SDL_assert(input.mFunction != nullptr);

		input.mFunction(scene, entity, keystate, delta);
	}

	updateMouse(scene, delta);
}

void InputSystem::updateMouse(Scene* scene, const float delta) {
	static std::unordered_map<Components::block::BlockType, std::uint64_t> BREAK_TIMES = {
		{Components::block::BlockType::AIR, 0},
		{Components::block::BlockType::DIRT, 20},
		{Components::block::BlockType::GRASS_BLOCK, 20},
		{Components::block::BlockType::STONE, 80},
	};

	// From Topright
	float x = 0, y = 0;
	const SDL_MouseButtonFlags flags = SDL_GetMouseState(&x, &y);

	// Convert Y to opengl cords
	const auto windowSize = mGame->getSystemManager()->getDemensions();
	y = windowSize.y() - y;

	const bool leftClick = flags & SDL_BUTTON_MASK(1);

	const auto playerPos = scene->get<Components::position>(mGame->getPlayerID()).mPosition;
	const int realX = static_cast<float>(x + playerPos.x() - windowSize.x() / 2) / Components::block::BLOCK_SIZE;
	const int realY = static_cast<float>(y + playerPos.y() - windowSize.y() / 2) / Components::block::BLOCK_SIZE;
	const Eigen::Vector2i blockPos{realX, realY};

	if (leftClick) {
		mPressLength += delta;

		if (mPressLength >= LONG_PRESS_ACTIVATION_TIME) {
			for (const auto [entity, block, texture] :
			     scene->view<Components::block, Components::texture>().each()) {
				SDL_Log("Overlap at %d %d", realX, realY);

				if (block.mPosition == blockPos && (mPressLength * 20) >= BREAK_TIMES[block.mType]) {
					scene->erase(entity);

					mPressLength = 0;

					break;
				}
			}
		}
	}

	if (!leftClick && mPressLength > 0 && mPressLength < LONG_PRESS_ACTIVATION_TIME) {
	}

	// TODO: Control options
	// TODO: Pickaxe cursor

	// shortclick = place
	// longclick = brake

	// Reset the length after processing
	if (!leftClick) {
		mPressLength = 0;
	}
}

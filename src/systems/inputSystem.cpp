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
	const static std::unordered_map<Components::block::BlockType, std::uint64_t> BREAK_TIMES = {
		{Components::block::BlockType::AIR, 0},
		{Components::block::BlockType::DIRT, 20},
		{Components::block::BlockType::STONE, 80},
	};

	(void)scene;
	(void)BREAK_TIMES;

	// From Topright
	float x = 0, y = 0;
	const SDL_MouseButtonFlags flags = SDL_GetMouseState(&x, &y);

	// Convert Y to opengl cords
	y = mGame->getDemensions().y() - y;

	const bool leftClick = flags & SDL_BUTTON_MASK(1);

	float realX = 0, realY = 0;

	if (leftClick) {
		mPressLength += delta;

		if (mPressLength >= LONG_PRESS_ACTIVATION_TIME) {
			for (const auto [entity, block, texture] :
			     scene->view<Components::block, Components::texture>().each()) {
				const auto textureSize = texture.mTexture->getSize();
				const auto bx = block.mPosition.x() * textureSize.x();
				const auto by = block.mPosition.y() * textureSize.y();
				// FIXME: Better collisions
				if ((realX <= bx || realX >= bx + textureSize.x()) || (realY <= by || realY >= by + textureSize.y())) {
					// Not colliding with the mouse
					continue;
				}

				scene->erase(entity);

				break;
			}
		}
	}

	SDL_assert(mPressLength >= 0 && "Hey! Why is the press length negative?");
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

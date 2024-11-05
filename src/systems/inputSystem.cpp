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
	constexpr const static std::unordered_map<Components::block::BlockType, std::uint64_t> BREAK_TIMES = {
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

	if (leftClick) {
		mPressLength += delta;

		if (mPressLength >= LONG_PRESS_ACTIVATION_TIME) {
			// SDL_Log("Unhandled long click for %f", mPressLength);
			// TODO: Implement
			for (const auto [entity, block, position, texture] :
			     scene->view<Components::block, Components::position, Components::texture>().each()) {
				// FIXME: Better collisions
				if ((x <= position.mPosition.x() ||
				     x >= position.mPosition.x() + texture.mTexture->getWidth()) ||
				    (y <= position.mPosition.y() ||
				     y >= position.mPosition.y() + texture.mTexture->getHeight())) {
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
		// FIXME: Better collisions
		for (const auto [entity, block, position, texture] :
		     scene->view<Components::block, Components::position, Components::texture>().each()) {
			if ((x <= position.mPosition.x() ||
			     x >= position.mPosition.x() + texture.mTexture->getWidth() * texture.mScale) ||
			    (y <= position.mPosition.y() ||
			     y >= position.mPosition.y() + texture.mTexture->getHeight() * texture.mScale)) {
				continue;
			}

			scene->erase(entity);

			// NOTE: Maybe debug test if we clicked on multiple entities?
			break;
		}
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

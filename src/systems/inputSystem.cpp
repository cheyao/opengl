#include "systems/inputSystem.hpp"

#include "opengl/texture.hpp"
#include "components.hpp"
#include "game.hpp"
#include "misc/sparse_set_view.hpp"
#include "scene.hpp"

#include <SDL3/SDL.h>
#include <functional>

InputSystem::InputSystem(class Game* game) : mGame(game), mPressedX(0), mPressedY(0), mPressLength(0) {}

void InputSystem::update(Scene* scene, const float delta) {
	const bool* keystate = mGame->getKeystate();

	for (const auto& [entity, input] : scene->view<Components::input>().each()) {
		SDL_assert(input.mFunction != nullptr);

		input.mFunction(scene, entity, keystate, delta);
	}

	updateMouse(scene, delta);
}

void InputSystem::updateMouse(class Scene* scene, const float delta) {
	(void)scene;

	// From Topright
	float x = 0;
	float y = 0;
	const SDL_MouseButtonFlags flags = SDL_GetMouseState(&x, &y);
	const bool leftClick = flags & SDL_BUTTON(1);

	if (leftClick) {
		mPressLength += delta;

		if (mPressLength >= LONG_PRESS_ACTIVATION_TIME) {
			SDL_Log("Long click for %f", mPressLength);
		}
	}

	SDL_assert(mPressLength >= 0 && "Hey! Why is the press length negative?");
	if (!leftClick && mPressLength > 0 && mPressLength < LONG_PRESS_ACTIVATION_TIME) {
		for (const auto [entity, block, position, texture] :
		     scene->view<Components::block, Components::position, Components::texture>().each()) {
			// FIXME: Better collisions
			// FIXME: Scaling
			if (x < position.mPosition.x() ||
			    x > position.mPosition.x() + texture.mTexture->getWidth()) {
				continue;
			}

			SDL_Log("Clicked on enitty %llu", entity);

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

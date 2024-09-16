#include "systems/inputSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "scene.hpp"
#include "misc/sparse_set_view.hpp"

#include <functional>
#include <SDL3/SDL.h>

InputSystem::InputSystem(class Game* game) : mGame(game), mPressLength(0) {}

void InputSystem::update(Scene* scene, const float delta) {
	const bool* keystate = mGame->getKeystate();

	for (const auto& [entity, input] : scene->view<Components::input>().each()) {
		SDL_assert(input.mFunction != nullptr);

		input.mFunction(scene, entity, keystate, delta);
	}

	updateMouse(scene, delta);
}

#define TO_MS(number) (number / 1000.0f)

void InputSystem::updateMouse(class Scene* scene, const float delta) {
	// From Topright
	float x = 0;
	float y = 0;
	const SDL_MouseButtonFlags flags = SDL_GetMouseState(&x, &y);
	const bool leftClick = flags & SDL_BUTTON(1);
	
	if (leftClick) {
		mPressLength += delta;

		if (mPressLength >= TO_MS(LONG_PRESS_ACTIVATION_TIME)) {
			SDL_Log("Long click for %f", mPressLength);
		}
	}

	SDL_assert(mPressLength >= 0 && "Hey! Why is the press length negative?");
	if (!leftClick && mPressLength > 0 && mPressLength < TO_MS(LONG_PRESS_ACTIVATION_TIME)) {
		SDL_Log("Short click");
		(void) scene;
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

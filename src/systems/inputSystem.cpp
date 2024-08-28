#include "systems/inputSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "scene.hpp"
#include "misc/sparse_set_view.hpp"

#include <functional>
#include <SDL3/SDL.h>

InputSystem::InputSystem(class Game* game) : mGame(game) {}

void InputSystem::update(Scene* scene, const float delta) {
	const bool* keystate = mGame->getKeystate();

	for (const auto& [entity, input] : scene->view<Components::input>().each()) {
		SDL_assert(input.function != nullptr);

		input.function(scene, entity, keystate, delta);
	}
}

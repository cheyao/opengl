#include "systems/inputSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "scene.hpp"

InputSystem::InputSystem(class Game* game) : mGame(game) {}

void InputSystem::update(Scene* scene, const float delta) {
	const bool* keystate = mGame->getKeystate();

	for (const auto& [entity, input] : scene->view<Components::input>().each()) {
		input.function(scene, entity, keystate, delta);
	}
}

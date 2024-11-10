#include "systems/UISystem.hpp"

#include "game.hpp"
#include "scene.hpp"
#include "screens/screen.hpp"

UISystem::UISystem(Game* game) : mGame(game) {}

void UISystem::update(Scene* scene, const float delta) {
	for (const auto& screen : mScreenStack) {
		screen->update(scene, delta);
	}
}

void UISystem::draw(Scene* scene) {
	for (const auto& screen : mScreenStack) {
		screen->draw(scene);
	}
}

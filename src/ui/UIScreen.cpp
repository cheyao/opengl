#include "ui/UIScreen.hpp"

#include "game.hpp"
#include "imgui.h"
#include "ui/UIComponent.hpp"

UIScreen::UIScreen(Game* game, const std::string name) : mGame(game), mState(UIScreen::ACTIVE), mName(name) {
	game->addUI(this);
}

UIScreen::~UIScreen() {
	mGame->removeUI(this);

	while (!mComponents.empty()) {
		delete mComponents.back();
	}
}

void UIScreen::update(const float delta) {
	updateScreen(delta);

#ifdef IMGUI
	ImGui::Begin("UI Components");
#endif
	for (const auto& component : mComponents) {
#ifdef IMGUI
		// TODO: Changable stuff (sliders)
		ImGui::Text("Component: %s position: %f %f", component->getName().data(), component->getPosition().x(), component->getPosition().y());
#endif
		component->update(delta);
	}
#ifdef IMGUI
	ImGui::End();
#endif
}

void UIScreen::updateScreen([[maybe_unused]] const float delta) {}

void UIScreen::draw(const Shader* shader) {
	for (const auto& component : mComponents) {
		component->draw(shader);
	}
}

void UIScreen::drawText(const Shader* shader) {
	for (const auto& component : mComponents) {
		component->drawText(shader);
	}
}

void UIScreen::processInput(const bool* keys) {
	for (const auto& component : mComponents) {
		component->input(keys);
	}
}

void UIScreen::touch(const SDL_FingerID& finger, const float x, const float y, const bool lift) {
	for (const auto& component : mComponents) {
		component->touch(finger, x, y, lift);
	}
}

void UIScreen::removeComponent(UIComponent* component) {
	const auto& iter = std::find(mComponents.begin(), mComponents.end(), component);

	[[likely]] if (iter != mComponents.end()) { mComponents.erase(iter); }
}

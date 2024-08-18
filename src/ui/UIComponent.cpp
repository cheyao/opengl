#include "ui/UIComponent.hpp"

#include "ui/UIScreen.hpp"

#include <SDL3/SDL.h>

UIComponent::UIComponent(UIScreen* owner) : mOwner(owner) { mOwner->addComponent(this); }

UIComponent::~UIComponent() { mOwner->removeComponent(this); }

void UIComponent::update([[maybe_unused]] const float delta) {}

void UIComponent::input([[maybe_unused]] const bool* keystate) {}

void UIComponent::draw([[maybe_unused]] const Shader* shader) {}
void UIComponent::drawText([[maybe_unused]] const Shader* shader) {}

void UIComponent::touch([[maybe_unused]] const SDL_FingerID& finger, [[maybe_unused]] const float x,
			[[maybe_unused]] const float y, [[maybe_unused]] const bool lift) {}

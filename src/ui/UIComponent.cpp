#include "ui/UIComponent.hpp"

#include "ui/UIScreen.hpp"

UIComponent::UIComponent(UIScreen* owner) : mOwner(owner) { mOwner->addComponent(this); }

UIComponent::~UIComponent() { mOwner->removeComponent(this); }

void UIComponent::update([[maybe_unused]] const float delta) {}

void UIComponent::input([[maybe_unused]] const bool* keystate) {}

void UIComponent::draw([[maybe_unused]] const Shader* shader) {}

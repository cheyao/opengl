#include "ui/UIComponent.hpp"

#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"

#include <SDL3/SDL.h>
#include <string_view>

UIComponent::UIComponent(UIScreen* owner, const Eigen::Vector2f position, const std::string_view& name)
	: mOwner(owner), mName(name), mPosition(position), mSize(Eigen::Vector2f::Zero()) {
	mOwner->addComponent(this);
}

UIComponent::~UIComponent() { mOwner->removeComponent(this); }

void UIComponent::update([[maybe_unused]] const float delta) {}

void UIComponent::input([[maybe_unused]] const bool* keystate) {}

void UIComponent::draw([[maybe_unused]] Shader* shader) {}

void UIComponent::drawText([[maybe_unused]] Shader* shader) {}

void UIComponent::touch([[maybe_unused]] const SDL_FingerID& finger, [[maybe_unused]] const float x,
			[[maybe_unused]] const float y, [[maybe_unused]] const bool lift) {}

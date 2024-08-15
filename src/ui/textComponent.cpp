#include "ui/textComponent.hpp"

#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include "game.hpp"

TextComponent::TextComponent(class UIScreen* owner) : UIComponent(owner) {}

void UIComponent::draw([[maybe_unused]] const Shader* shader) {
	const Shader* textShader = mOwner->getGame()->getShader("text.vert", "text.frag");
}

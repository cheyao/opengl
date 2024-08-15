#include "ui/textComponent.hpp"

#include "game.hpp"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include "ui/freetype.hpp"

TextComponent::TextComponent(class UIScreen* owner, std::string_view text) : UIComponent(owner), mText(text) {}

void UIComponent::draw([[maybe_unused]] const Shader* shader) {
	const Shader* textShader = mOwner->getGame()->getShader("text.vert", "text.frag");
	const auto& c = mOwner->getGame()->getGlyph(U'h');
	
}

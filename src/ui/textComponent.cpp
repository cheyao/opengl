#include "ui/textComponent.hpp"

#include "game.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include "ui/freetype.hpp"

// TODO: Size change
TextComponent::TextComponent(class UIScreen* owner, std::string_view text) : UIComponent(owner), mText(text) {
}

void TextComponent::draw([[maybe_unused]] const Shader* shader) {
	mOwner->getGame()->getFontManager()->drawGlyph(U'h', mOwner->getGame());
}

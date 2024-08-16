#include "ui/textComponent.hpp"

#include "game.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include "ui/freetype.hpp"

#include <wchar.h>

// TODO: Size change
TextComponent::TextComponent(class UIScreen* owner, std::string_view text) : UIComponent(owner), mText(text) {}

// FIXME: Newlines
void TextComponent::drawText([[maybe_unused]] const Shader* shader) {
	FontManager* manager = mOwner->getGame()->getFontManager();

	Eigen::Vector2f offset = Eigen::Vector2f::Zero();

	for (const char32_t c : mText) {
		manager->drawGlyph(c, shader, offset);

		offset += manager->getOffset(c);
	}
}

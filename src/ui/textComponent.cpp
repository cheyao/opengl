#include "ui/textComponent.hpp"

#include "game.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include "ui/freetype.hpp"

#include <wchar.h>

// TODO: Size change
TextComponent::TextComponent(class UIScreen* owner, const std::basic_string<char32_t>& text, Eigen::Vector2f position) : UIComponent(owner), mText(text), mPosition(position) {
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

// FIXME: Newlines
// TODO: Scaling
void TextComponent::drawText([[maybe_unused]] const Shader* shader) {
	FontManager* manager = mOwner->getGame()->getFontManager();

	// PERF: Only calculate if center
	Eigen::Vector2f size = Eigen::Vector2f::Zero();
	for (const char32_t c : mText) {
		size.x() += manager->getOffset(c).x();
		size.y() = std::max(size.y(), manager->getSize(c).y());
	}

	Eigen::Vector2f offset = mPosition;

	if (offset.x() == CENTER) {
		offset.x() = static_cast<float>(mOwner->getGame()->getWidth()) / 2 -
			  static_cast<float>(size.x()) / 2;
	} else if (offset.x() < 0) {
		offset.x() += mOwner->getGame()->getWidth();
	}

	if (offset.y() == CENTER) {
		offset.y() = static_cast<float>(mOwner->getGame()->getHeight()) / 2 -
			  (static_cast<float>(size.y())) / 2;
	} else {
		if (offset.y() < 0) {
			offset.y() += mOwner->getGame()->getHeight();
		}
	}

	for (const char32_t c : mText) {
		manager->drawGlyph(c, shader, offset);

		offset += manager->getOffset(c);
	}
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

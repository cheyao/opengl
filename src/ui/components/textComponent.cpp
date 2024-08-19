#include "ui/components/textComponent.hpp"

#include "game.hpp"
#include "managers/fontManager.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <wchar.h>

// TODO: Size change
TextComponent::TextComponent(class UIScreen* owner, class UIComponent* parent, const std::string& id,
			     const Eigen::Vector2f offset)
	: UIComponent(owner, offset, "TextComponent " + id), mTextID(id), mParent(parent), mOffset(offset) {
	assert(owner != nullptr);

	FontManager* manager = mOwner->getGame()->getFontManager();
	Eigen::Vector2f size = Eigen::Vector2f::Zero();
	for (const char32_t c : mOwner->getGame()->getLocaleManager()->get(mTextID)) {
		size.x() += manager->getOffset(c).x();
		size.y() = std::max(size.y(), manager->getSize(c).y());
	}
	mSize = size;
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
	Eigen::Vector2f offset = mOffset;

	if (offset.x() == CENTER || offset.y() == CENTER) {
		Eigen::Vector2f size = Eigen::Vector2f::Zero();
		for (const char32_t c : mOwner->getGame()->getLocaleManager()->get(mTextID)) {
			size.x() += manager->getOffset(c).x();
			size.y() = std::max(size.y(), manager->getSize(c).y());
		}
		mSize = size;
	}

	Eigen::Vector2f parentPos = mParent == nullptr ? Eigen::Vector2f::Zero() : mParent->getPosition();
	Eigen::Vector2f parentSize =
		mParent == nullptr ? Eigen::Vector2f(mOwner->getGame()->getWidth(), mOwner->getGame()->getHeight())
				   : mParent->getSize();

	if (parentPos.x() == CENTER) {
		parentPos.x() = static_cast<float>(mOwner->getGame()->getWidth()) / 2 - parentSize.x() / 2;
	}
	if (parentPos.y() == CENTER) {
		parentPos.y() = static_cast<float>(mOwner->getGame()->getHeight()) / 2 - parentSize.y() / 2;
	}

	if (offset.x() == CENTER) {
		offset.x() = parentPos.x() + parentSize.x() / 2 - mSize.x() / 2;
	} else if (offset.x() < 0) {
		offset.x() += parentPos.x() + parentSize.x();
	} else {
		offset.x() += parentPos.x();
	}

	if (offset.y() == CENTER) {
		offset.y() = parentPos.y() + parentSize.y() / 2 - mSize.y() / 2;
	} else if (offset.y() < 0) {
		offset.y() = parentPos.y() - offset.y();
	} else {
		offset.y() = parentPos.y() + offset.y();
	}

	for (const char32_t c : mOwner->getGame()->getLocaleManager()->get(mTextID)) {
		manager->drawGlyph(c, shader, offset);

		offset += manager->getOffset(c);
	}
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

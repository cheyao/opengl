#include "ui/screens/divUI.hpp"

#include "game.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

DivUI::DivUI(class Game* game, class UIScreen* parent) : UIScreen(game, "DivUI"), mParent(parent) {}

DivUI::~DivUI() {
	// FIXME: The parent gets deleted one frame later
	mParent->setState(UIScreen::DEAD);
}

void DivUI::updateScreen([[maybe_unused]] const float delta) {
	Eigen::Vector2f size = Eigen::Vector2f::Zero();

	for (const auto& component : mComponents) {
		size.x() += std::max(size.x(), component->getSize().x());
		size.y() += component->getSize().y();
	}

	const float yOffset = getGame()->getHeight();

	for (size_t i = 0; i < mComponents.size(); ++i) {
		Eigen::Vector2f position = Eigen::Vector2f::Zero();

		position.x() = CENTER;
		position.y() = size.y();

		mComponents[i]->setPosition();
	}
}


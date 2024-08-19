#include "ui/screens/divUI.hpp"

#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/UIComponent.hpp"

DivUI::DivUI(class Game* owner, class UIScreen* parent) : UIScreen(owner), mParent(parent) {}

DivUI::~DivUI() {
	// FIXME: The parent gets deleted one frame later
	mParent->setState(UIScreen::DEAD);
	SDL_Log("NO");
}

void DivUI::updateScreen([[maybe_unused]] const float delta) {
	// Position the items
	Eigen::Vector2f size;

	for (const auto& component : mComponents) {
		size.y() += component->
	}
}

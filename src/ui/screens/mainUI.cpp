#include "ui/screens/mainUI.hpp"

#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/components/backgroundComponent.hpp"
#include "ui/components/buttonComponent.hpp"
#include "ui/components/textComponent.hpp"
#include "utils.hpp"

MainUI::MainUI(class Game* game) : UIScreen(game) {
	game->setPause(true);
	game->getRenderer()->setWindowRelativeMouseMode(SDL_FALSE);

	new BackgroundComponent(this, Eigen::Vector3f(1.0f, 0.0f, 0.0f));

	ButtonComponent* start = new ButtonComponent(this, getGame()->getTexture("ui" SEPARATOR "start.png"),
						     Eigen::Vector2f(CENTER, CENTER));

	start->onClick([this] {
		this->getGame()->setPause(false);
		this->getGame()->removeUI(this);
		this->getGame()->getRenderer()->setWindowRelativeMouseMode(SDL_TRUE);
		this->mState = UIScreen::DEAD;
	});

	new TextComponent(this, "start_game", Eigen::Vector2f(CENTER, CENTER));
}

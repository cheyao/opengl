#include "ui/screens/mainUI.hpp"

#include "game.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/components/backgroundComponent.hpp"
#include "ui/components/buttonComponent.hpp"
#include "ui/components/textComponent.hpp"
#include "ui/screens/divUI.hpp"
#include "utils.hpp"

MainUI::MainUI(class Game* const game) : UIScreen(game, "MainUI") {
	/*
	game->setPause(true);
	game->getRenderer()->setWindowRelativeMouseMode(SDL_FALSE);

	new BackgroundComponent(this, Eigen::Vector3f(1.0f, 0.0f, 0.0f));

	DivUI* const div = new DivUI(game, this);

	ButtonComponent* const quit = new ButtonComponent(div, getGame()->getTexture("ui" SEPARATOR "start.png"),
							  Eigen::Vector2f(CENTER, CENTER));

	quit->onRelease([div] { div->getGame()->quit(); });

	ButtonComponent* const settings = new ButtonComponent(div, getGame()->getTexture("ui" SEPARATOR "start.png"),
							      Eigen::Vector2f(CENTER, CENTER));

	settings->onRelease([div] { (void)div; });

	ButtonComponent* const start = new ButtonComponent(div, getGame()->getTexture("ui" SEPARATOR "start.png"),
							   Eigen::Vector2f(CENTER, CENTER));

	start->onRelease([div] {
		div->getGame()->setPause(false);
		div->getGame()->getRenderer()->setWindowRelativeMouseMode(SDL_TRUE);
		div->setState(UIScreen::DEAD);
	});

	new TextComponent(div, quit, "quit_game", Eigen::Vector2f(CENTER, CENTER));
	new TextComponent(div, start, "start_game", Eigen::Vector2f(CENTER, CENTER));
	new TextComponent(div, settings, "settings", Eigen::Vector2f(CENTER, CENTER));
	*/
}

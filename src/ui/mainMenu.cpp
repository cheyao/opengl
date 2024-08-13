#include "ui/mainMenu.hpp"

#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/backgroundComponent.hpp"
#include "ui/buttonComponent.hpp"
#include "utils.hpp"

#include <cmath>

MainMenu::MainMenu(class Game* game) : UIScreen(game) {
	game->setPause(true);
	game->getRenderer()->setWindowRelativeMouseMode(0);

	new BackgroundComponent(this, Eigen::Vector3f(0.5f, 0.5f, 0.5f));

	ButtonComponent* start =
		new ButtonComponent(this, getGame()->getTexture("ui" SEPARATOR "start.png"), Eigen::Vector2f(NAN, NAN));

	start->onClick([this, game] {
		game->removeUI(this);
		game->getRenderer()->setWindowRelativeMouseMode(1);
		game->setPause(false);
	});
}

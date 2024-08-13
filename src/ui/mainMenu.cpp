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
	game->getRenderer()->setWindowRelativeMouseMode(SDL_FALSE);
	game->setPause(true);

	new BackgroundComponent(this, Eigen::Vector3f(0.5f, 0.5f, 0.5f));

	ButtonComponent* start =
		new ButtonComponent(this, getGame()->getTexture("ui" SEPARATOR "start.png"), Eigen::Vector2f(NAN, NAN));

	start->onClick([this, game] {
		game->getRenderer()->setWindowRelativeMouseMode(SDL_TRUE);
		game->setPause(false);
		game->removeUI(this);
	});
}

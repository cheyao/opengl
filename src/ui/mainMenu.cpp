#include "ui/mainMenu.hpp"

#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/backgroundComponent.hpp"
#include "ui/buttonComponent.hpp"
#include "utils.hpp"

MainMenu::MainMenu(class Game* game) : UIScreen(game) {
	game->setPause(true);
	game->getRenderer()->setWindowRelativeMouseMode(SDL_FALSE);

	new BackgroundComponent(this, Eigen::Vector3f(1.0f, 0.0f, 0.0f));

	ButtonComponent* start =
		new ButtonComponent(this, getGame()->getTexture("ui" SEPARATOR "start.png"), Eigen::Vector2f(CENTER, CENTER));

	start->onClick([this, game] {
		game->setPause(false);
		game->removeUI(this);
		game->getRenderer()->setWindowRelativeMouseMode(SDL_TRUE);
	});
}

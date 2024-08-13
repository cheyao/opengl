#include "ui/mainMenu.hpp"

#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/buttonComponent.hpp"
#include "utils.hpp"

#include <cmath>

MainMenu::MainMenu(class Game* game) : UIScreen(game) {
	ButtonComponent* start =
		new ButtonComponent(this, getGame()->getTexture("ui" SEPARATOR "start.png"), Eigen::Vector2f(NAN, NAN));

	start->onClick([this, game] {
		game->removeUI(this);
		game->getRenderer()->setWindowRelativeMouseMode(1);
	});
}

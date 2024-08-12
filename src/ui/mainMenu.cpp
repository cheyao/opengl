#include "ui/mainMenu.hpp"

#include "game.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/buttonComponent.hpp"

#include <cmath>

MainMenu::MainMenu(class Game* game) : UIScreen(game) {
	new ButtonComponent(this, getGame()->getTexture("up.png"), Eigen::Vector2f(1024 - 64, NAN));
}

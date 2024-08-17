#include "ui/screens/controlUI.hpp"

#include "game.hpp"
#include "ui/UIScreen.hpp"
#include "ui/components/buttonComponent.hpp"

#include <SDL3/SDL.h>

ControlUI::ControlUI(Game* game) : UIScreen(game) {
	constexpr const int padding = 20;
	constexpr const int buttonSize = 64;

	ButtonComponent* up =
		new ButtonComponent(this, game->getTexture("ui" SEPARATOR "up.png"),
				    Eigen::Vector2f(3 * padding + buttonSize, 5 * -padding + 2 * -buttonSize));
	up->onClick([game] { game->setKey(SDL_SCANCODE_W, true); });
	up->onRelease([game] { game->setKey(SDL_SCANCODE_W, false); });

	ButtonComponent* down = new ButtonComponent(this, game->getTexture("ui" SEPARATOR "down.png"),
						    Eigen::Vector2f(3 * padding + buttonSize, -padding));
	down->onClick([game] { game->setKey(SDL_SCANCODE_S, true); });
	down->onRelease([game] { game->setKey(SDL_SCANCODE_S, false); });

	ButtonComponent* left = new ButtonComponent(this, game->getTexture("ui" SEPARATOR "left.png"),
						    Eigen::Vector2f(padding, 3 * -padding + -buttonSize));
	left->onClick([game] { game->setKey(SDL_SCANCODE_A, true); });
	left->onRelease([game] { game->setKey(SDL_SCANCODE_A, false); });

	ButtonComponent* right =
		new ButtonComponent(this, game->getTexture("ui" SEPARATOR "right.png"),
				    Eigen::Vector2f(5 * padding + 2 * buttonSize, 3 * -padding + -buttonSize));
	right->onClick([game] { game->setKey(SDL_SCANCODE_D, true); });
	right->onRelease([game] { game->setKey(SDL_SCANCODE_D, false); });
}

ControlUI::~ControlUI() {}

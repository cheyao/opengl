#include "ui/controlUI.hpp"

#include "game.hpp"
#include "ui/UIScreen.hpp"
#include "ui/buttonComponent.hpp"

#include <SDL3/SDL.h>

ControlUI::ControlUI(Game* game) : UIScreen(game) {
	/*
	game->getTexture("up.png");
	game->getTexture("down.png");
	game->getTexture("left.png");
	game->getTexture("right.png");
	*/

	constexpr const int padding = 20;
	constexpr const int buttonSize = 64;

	ButtonComponent* up =
		new ButtonComponent(this, game->getTexture("up.png"),
				    Eigen::Vector2f(3 * padding + buttonSize, 5 * -padding + 2 * -buttonSize));
	up->onClick([](Game* game) {
		game->setKey(SDL_SCANCODE_W, true);
	});
	up->onRelease([](Game* game) {
		game->setKey(SDL_SCANCODE_W, false);
	});

	ButtonComponent* down = new ButtonComponent(this, game->getTexture("down.png"),
						    Eigen::Vector2f(3 * padding + buttonSize, -padding));
	down->onClick([](Game* game) {
		game->setKey(SDL_SCANCODE_S, true);
	});
	down->onRelease([](Game* game) {
		game->setKey(SDL_SCANCODE_S, false);
	});

	ButtonComponent* left = new ButtonComponent(this, game->getTexture("left.png"),
						    Eigen::Vector2f(padding, 3 * -padding + -buttonSize));
	left->onClick([](Game* game) {
		game->setKey(SDL_SCANCODE_A, true);
	});
	left->onRelease([](Game* game) {
		game->setKey(SDL_SCANCODE_A, false);
	});

	ButtonComponent* right =
		new ButtonComponent(this, game->getTexture("right.png"),
				    Eigen::Vector2f(5 * padding + 2 * buttonSize, 3 * -padding + -buttonSize));
	right->onClick([](Game* game) {
		game->setKey(SDL_SCANCODE_D, true);
	});
	right->onRelease([](Game* game) {
		game->setKey(SDL_SCANCODE_D, false);
	});
}

ControlUI::~ControlUI() {}

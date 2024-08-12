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
	new ButtonComponent(this, game->getTexture("up.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(3 * padding + buttonSize, 5 * -padding + 2 * -buttonSize));
	// new ButtonComponent(this, game->getTexture("down.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(3 * padding + buttonSize, -padding));
	// new ButtonComponent(this, game->getTexture("up.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(padding, 3 * -padding + -buttonSize));
	// new ButtonComponent(this, game->getTexture("right.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(5 * padding + 2 * buttonSize, 3 * -padding + -buttonSize));
}

ControlUI::~ControlUI() {}

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

	constexpr const int buttonSize = 64;
	new ButtonComponent(this, game->getTexture("down.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(buttonSize, -buttonSize));
	/*
	new ButtonComponent(this, game->getTexture("up.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(50 + buttonSize, -50));
	new ButtonComponent(this, game->getTexture("left.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(25, -30));
	new ButtonComponent(this, game->getTexture("right.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(75, -30));
	*/
}

ControlUI::~ControlUI() {}

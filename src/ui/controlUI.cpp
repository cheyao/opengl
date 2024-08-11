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
	new ButtonComponent(this, game->getTexture("up.png"), [] { SDL_Log("Clicked!"); }, Eigen::Vector2f(10.0f, -10.0f));
}

ControlUI::~ControlUI() {}

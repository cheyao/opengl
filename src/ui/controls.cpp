#include "ui/controls.hpp"

#include "game.hpp"
#include "ui/UIScreen.hpp"

Controls::Controls(Game* game) : UIScreen(game) {
	game->getTexture("up.png");
	game->getTexture("down.png");
	game->getTexture("left.png");
	game->getTexture("right.png");
}

Controls::~Controls() {}

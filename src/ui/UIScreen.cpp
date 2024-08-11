#include "ui/UIScreen.hpp"

#include "game.hpp"
#include "opengl/texture.hpp"

UIScreen::UIScreen(Game* game) : mGame(game) { game->addUI(this); 
}

UIScreen::~UIScreen() { mGame->removeUI(this); }

void update([[maybe_unused]] float deltaTime) {}

void draw([[maybe_unused]] Shader* shader) {}

void processInput([[maybe_unused]] const bool* keys) {}

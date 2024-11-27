#include "screens/screen.hpp"

#include "game.hpp"

Screen::Screen(Game* game) : mGame(game) {}

bool Screen::update(class Scene*, float) { return true; }

void Screen::draw(class Scene*) {}

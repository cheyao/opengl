#include "screens/screen.hpp"

#include "game.hpp"

Screen::Screen(Game* game) : mGame(game) {}

bool Screen::update([[maybe_unused]] class Scene* scene, [[maybe_unused]] float delta) {}

void Screen::draw([[maybe_unused]] class Scene* scene) {}

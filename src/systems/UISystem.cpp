#include "systems/UISystem.hpp"

#include "game.hpp"
#include "scene.hpp"

UISystem::UISystem(Game* game) : mGame(game) {}

void UISystem::draw(Scene* scene) { (void)scene; }

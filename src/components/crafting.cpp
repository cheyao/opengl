#include "components/crafting.hpp"

#include <SDL3/SDL.h>

Crafting::Crafting(class Game* game, std::uint64_t row, std::uint64_t col)
	: Screen(game), mGame(game), mGrid(row * col) {}

bool Crafting::update(class Scene*, float) {
	SDL_Log("Craft");

	// Solve the mouse
	return false;
}

void Crafting::draw(class Scene*) { (void)mGame; }

#include "components/crafting.hpp"

Crafting::Crafting(class Game* game, std::uint64_t row, std::uint64_t col)
	: Screen(game), mGame(game), mGrid(row * col) {}

bool Crafting::update(class Scene*, float) {
	// Solve the mouse

	return false;
}

void Crafting::draw(class Scene*) { (void)mGame; }

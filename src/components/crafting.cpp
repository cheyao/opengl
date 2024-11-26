#include "components/crafting.hpp"

Crafting::Crafting(class Game* game, std::uint64_t row, std::uint64_t col)
	: Screen(game), mGame(game), mGrid(row * col) {}

bool Crafting::update(class Scene* scene, float delta) {}

void Crafting::draw(class Scene* scene) {}

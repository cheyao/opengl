#include "components/crafting.hpp"
#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <cstdint>

CraftingInventory::CraftingInventory(class Game* game, std::uint64_t size, EntityID entity, std::uint64_t row,
				     std::uint64_t col)
	: Inventory(game, size, entity), mRows(row), mCols(col) {}

CraftingInventory::CraftingInventory(class Game* game, const nlohmann::json& contents, EntityID entity,
				     std::uint64_t row, std::uint64_t col)
	: Inventory(game, contents, entity), mRows(row), mCols(col) {}

bool CraftingInventory::update(class Scene* scene, float delta) {
	SDL_Log("Craft");
	(void)mRows;
	(void)mCols;

	return Inventory::update(scene, delta);
}

void CraftingInventory::draw(class Scene* scene) { Inventory::draw(scene); }

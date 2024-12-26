#include "components/crafting.hpp"
#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <cstdint>

CraftingInventory::CraftingInventory(class Game* game, std::uint64_t size, EntityID entity, std::uint64_t row,
				     std::uint64_t col)
	: Inventory(game, size, entity), mGrid(row * col) {}

CraftingInventory::CraftingInventory(class Game* game, const nlohmann::json& contents, EntityID entity,
				     std::uint64_t row, std::uint64_t col)
	: Inventory(game, contents, entity), mGrid(row * col) {}

bool CraftingInventory::update(class Scene* scene, float delta) {
	SDL_Log("Craft");

	return Inventory::update(scene, delta);
}

void CraftingInventory::draw(class Scene* scene) { Inventory::draw(scene); }

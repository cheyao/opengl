#include "components/playerInventory.hpp"

PlayerInventory::PlayerInventory(class Game* game, std::size_t size, EntityID entity) : Inventory(game, size, entity) {}
PlayerInventory::PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Inventory(game, contents, entity) {}

#pragma once

#include "components/inventory.hpp"

class PlayerInventory : public Inventory {
	explicit PlayerInventory(class Game* game, std::size_t size, EntityID entity = 0);
	explicit PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity = 0);
	PlayerInventory(PlayerInventory&&) = delete;
	PlayerInventory(const PlayerInventory&) = delete;
	PlayerInventory& operator=(PlayerInventory&&) = delete;
	PlayerInventory& operator=(const PlayerInventory&) = delete;
	~PlayerInventory() = default;
};

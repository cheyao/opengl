#pragma once

#include "components.hpp"
#include "components/inventory.hpp"
#include "screens/screen.hpp"

#include <cstdint>

class CraftingInventory : public Inventory {
      public:
	explicit CraftingInventory(class Game* game, std::uint64_t size, EntityID entity, std::uint64_t row,
				   std::uint64_t col);
	explicit CraftingInventory(class Game* game, const nlohmann::json& contents, EntityID entity, std::uint64_t row,
				   std::uint64_t col);
	CraftingInventory(CraftingInventory&&) = delete;
	CraftingInventory(const CraftingInventory&) = delete;
	CraftingInventory& operator=(CraftingInventory&&) = delete;
	CraftingInventory& operator=(const CraftingInventory&) = delete;
	~CraftingInventory() override = default;

	bool update(class Scene* scene, float delta) override;
	void draw(class Scene* scene) override;

      private:
	std::vector<Components::Item> mGrid;
};

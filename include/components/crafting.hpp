#pragma once

#include "components.hpp"
#include "components/inventory.hpp"
#include "screens/screen.hpp"
#include "third_party/rapidjson/fwd.h"

#include <cstdint>

class CraftingInventory : public Inventory {
      public:
	explicit CraftingInventory(class Game* game, std::uint64_t size, EntityID entity, std::uint64_t row,
				   std::uint64_t col);
	explicit CraftingInventory(class Game* game, const rapidjson::Value& contents, EntityID entity, std::uint64_t row,
				   std::uint64_t col);
	CraftingInventory(CraftingInventory&&) = delete;
	CraftingInventory(const CraftingInventory&) = delete;
	CraftingInventory& operator=(CraftingInventory&&) = delete;
	CraftingInventory& operator=(const CraftingInventory&) = delete;
	~CraftingInventory() override = default;

	bool update(class Scene* scene, float delta) override;
	void draw(class Scene* scene) override;

      private:
	std::uint64_t mRows;
	std::uint64_t mCols;
	std::vector<Components::Item> mCraftingItems;
	std::vector<std::uint64_t> mCraftingCount;
};

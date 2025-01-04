#pragma once

#include "components.hpp"
#include "components/inventory.hpp"
#include "screens/screen.hpp"

#include <cstdint>

struct furnace_t {};

class FurnaceInventory : public Inventory {
      public:
	explicit FurnaceInventory(struct furnace_t);
	FurnaceInventory(FurnaceInventory&&) = delete;
	FurnaceInventory(const FurnaceInventory&) = delete;
	FurnaceInventory& operator=(FurnaceInventory&&) = delete;
	FurnaceInventory& operator=(const FurnaceInventory&) = delete;
	~FurnaceInventory() override = default;

	bool update(class Scene* scene, float delta) override;
	void tick(class Scene* scene, float delta);
	void draw(class Scene* scene) override;

      private:
	void placeGrid();
	bool checkRecipie(std::uint64_t r);

	std::vector<Components::Item> mSmeltingItems;
	std::vector<std::uint64_t> mSmeltingCount;

	double mFuelLeft;
	double mRecipieTime;

	Components::Item mLastCraft;

	constexpr const static inline double mFuelOffsetX = 55;
	constexpr const static inline double mFuelOffsetY = 98;
	constexpr const static inline double mBX = 0;
	constexpr const static inline double mBY = 36;
	constexpr const static inline double mOutX = 56 + INVENTORY_SLOT_X / 4;
	constexpr const static inline double mOutY = -22 + INVENTORY_SLOT_Y / 4;

	constexpr const static inline std::size_t FUEL_SLOT = 0;
	constexpr const static inline std::size_t COOK_SLOT = 1;
	constexpr const static inline std::size_t OUTPUT_SLOT = 2;
};

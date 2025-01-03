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
	void draw(class Scene* scene) override;

      private:
	void craft();
	void placeGrid();
	bool checkRecipie(std::uint64_t r);

	std::vector<Components::Item> mSmeltingItems;
	std::vector<std::uint64_t> mSmeltingCount;

	std::size_t mLastCraft;

	constexpr const static inline double mAX = 56;
	constexpr const static inline double mAY = 97;
	constexpr const static inline double mBX = 0;
	constexpr const static inline double mBY = 36;
	constexpr const static inline double mOutX = 57;
	constexpr const static inline double mOutY = 23;
};

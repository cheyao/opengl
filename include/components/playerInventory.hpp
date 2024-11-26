#pragma once

#include "components/inventory.hpp"

#include <SDL3/SDL_assert.h>
#include <cstddef>

class PlayerInventory : public Inventory {
      public:
	explicit PlayerInventory(class Game* game, std::size_t size, EntityID entity = 0);
	explicit PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity = 0);
	PlayerInventory(PlayerInventory&&) = delete;
	PlayerInventory(const PlayerInventory&) = delete;
	PlayerInventory& operator=(PlayerInventory&&) = delete;
	PlayerInventory& operator=(const PlayerInventory&) = delete;
	~PlayerInventory() override = default;

	bool update(class Scene* scene, float delta) override;
	void select(std::size_t cell) {
		SDL_assert(cell < 9 && cell >= 0);
		mSelect = cell;
	}
	std::size_t getSelection() const { return mSelect; }

      private:
	std::size_t mSelect;
};

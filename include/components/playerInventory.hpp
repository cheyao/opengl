#pragma once

#include "components/crafting.hpp"
#include "third_party/rapidjson/fwd.h"

#include <SDL3/SDL.h>
#include <cstddef>

class PlayerInventory : public CraftingInventory {
      public:
	explicit PlayerInventory(class Game* game, std::size_t size, EntityID entity = 0);
	explicit PlayerInventory(class Game* game, const rapidjson::Value& contents, EntityID entity = 0);
	PlayerInventory(PlayerInventory&&) = delete;
	PlayerInventory(const PlayerInventory&) = delete;
	PlayerInventory& operator=(PlayerInventory&&) = delete;
	PlayerInventory& operator=(const PlayerInventory&) = delete;
	~PlayerInventory() override = default;

	bool update(class Scene* scene, float delta) override;
	void draw(class Scene* scene) override;

	void select(std::size_t cell) {
		SDL_assert(cell < 9);
		mSelect = cell;
	}
	std::size_t getSelection() const { return mSelect; }
	void tryPlace(class Scene* scene, const Eigen::Vector2i& pos);

      private:
	std::size_t mSelect;
};

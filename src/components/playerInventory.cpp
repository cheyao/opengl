#include "components/playerInventory.hpp"

#include "components.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>
#include <cstddef>

PlayerInventory::PlayerInventory(class Game* game, std::size_t size, EntityID entity)
	: Inventory(game, size, entity), mSelect(0) {}
PlayerInventory::PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Inventory(game, contents, entity), mSelect(0) {}


void PlayerInventory::tryPlace(class Scene* scene, const Eigen::Vector2i& pos) {
	if (mItems[mSelect] == Components::Item::AIR) {
		return;
	}

	(void) scene;
	(void) pos;
}

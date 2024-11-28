#include "components/playerInventory.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"

#include <SDL3/SDL.h>
#include <cstddef>

PlayerInventory::PlayerInventory(class Game* game, std::size_t size, EntityID entity)
	: Inventory(game, size, entity), mSelect(0) {}
PlayerInventory::PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Inventory(game, contents, entity), mSelect(0) {}

bool PlayerInventory::update(class Scene* scene, float delta) {
	Inventory::update(scene, delta);

	// Handle mouse and keys
	const auto keystate = mGame->getKeystate();

	// Quit menu
	if (keystate[SDL_SCANCODE_ESCAPE]) {
		mGame->getSystemManager()->getUISystem()->pop();
	}

	float sx, sy;
	float ox, oy;
	float scale;

	SystemManager* systemManager = mGame->getSystemManager();
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	if (dimensions.x() <= dimensions.y()) {
		sx = dimensions.x() / 4 * 3;
		sy = sx / INVENTORY_TEXTURE_WIDTH * INVENTORY_TEXTURE_HEIGHT;
		ox = sx / 6;
		oy = (dimensions.y() - sy) / 2;

		scale = sx / INVENTORY_TEXTURE_WIDTH;
	} else {
		sy = dimensions.y() / 4 * 3;
		sx = sy / INVENTORY_TEXTURE_HEIGHT * INVENTORY_TEXTURE_WIDTH;
		ox = (dimensions.x() - sx) / 2;
		oy = sy / 6;

		scale = sy / INVENTORY_TEXTURE_HEIGHT;
	}

	ox += INVENTORY_SLOTS_OFFSET_X * scale - (INVENTORY_SLOT_X * scale / 2 - ox / INVENTORY_INV_SCALE);
	oy += INVENTORY_SLOTS_OFFSET_Y * scale - (INVENTORY_SLOT_Y * scale / 2 - oy / INVENTORY_INV_SCALE);

	float x, y;
	SDL_GetMouseState(&x, &y);
	y = dimensions.y() - y;

	// Not inside the space
	if (x < ox || y < oy || x > (ox + 9 * INVENTORY_SLOT_X * scale) || y > (oy + 4 * INVENTORY_SLOT_Y * scale)) {
		return true;
	}

	SDL_Log("In %f %f", x, y);

	return true;
}

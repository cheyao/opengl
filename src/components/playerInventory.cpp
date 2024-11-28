#include "components/playerInventory.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/eventManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
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
	float x, y;
	auto buttons = SDL_GetMouseState(&x, &y);

	if (!(buttons & SDL_BUTTON_LMASK)) {
		return true;
	}

	y = dimensions.y() - y;

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

	// Not inside the space
	if (x < ox || y < oy || x > (ox + 9 * INVENTORY_SLOT_X * scale) || y > (oy + 4 * INVENTORY_SLOT_Y * scale)) {
		return true;
	}

	x -= ox;
	y -= oy;

	int slot =
		static_cast<int>(x / (INVENTORY_SLOT_X * scale)) + static_cast<int>(y / (INVENTORY_SLOT_Y * scale)) * 9;

	SDL_Log("%d! %b", slot, scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL));
	if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) && mCount[slot] != 0) {
		scene->mMouse.item = mItems[slot];
		scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
		SDL_Log("Yes!");
	}

	return true;
}

void PlayerInventory::draw(class Scene* scene) {
	Inventory::draw(scene);

	if (scene->mMouse.item == Components::Item::AIR) {
		return;
	}

	SystemManager* systemManager = mGame->getSystemManager();
	Shader* shader = systemManager->getShader("ui.vert", "ui.frag");
	Mesh* mesh = systemManager->getUISystem()->getMesh();
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	float mx, my;
	SDL_GetMouseState(&mx, &my);
	my = dimensions.y() - my;

	shader->set("texture_diffuse"_u, 0);

	Texture* texture = systemManager->getTexture(registers::TEXTURES.at(scene->mMouse.item));
	shader->set("size"_u, static_cast<Eigen::Vector2f>(texture->getSize() / Inventory::INVENTORY_INV_SCALE));

	texture->activate(0);
	shader->set("offset"_u, mx - texture->getWidth() / Inventory::INVENTORY_INV_SCALE / 2,
		    my - texture->getHeight() / Inventory::INVENTORY_INV_SCALE / 2);
	mesh->draw(shader);
}

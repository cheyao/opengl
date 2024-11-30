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

	SystemManager* systemManager = mGame->getSystemManager();
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	float sx, sy;
	float ox, oy;
	float scale;

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

	ox += INVENTORY_SLOTS_OFFSET_X * scale - (INVENTORY_SLOT_X * scale / 2 - sx / INVENTORY_INV_SCALE);
	oy += INVENTORY_SLOTS_OFFSET_Y * scale - (INVENTORY_SLOT_Y * scale / 2 - sy / INVENTORY_INV_SCALE);

	float mouseX, mouseY;
	auto buttons = SDL_GetMouseState(&mouseX, &mouseY);

	if (!(buttons & SDL_BUTTON_LMASK)) {
		return true;
	}
	mouseY = dimensions.y() - mouseY;

	// Not inside the space
	if (mouseX < ox || mouseY < oy || mouseX > (ox + 9 * INVENTORY_SLOT_X * scale) ||
	    mouseY > (oy + 4 * INVENTORY_SLOT_Y * scale)) {
		return true;
	}

	mouseX -= ox;
	mouseY -= oy;

	int slot = static_cast<int>(mouseX / (INVENTORY_SLOT_X * scale)) +
		   static_cast<int>(mouseY / (INVENTORY_SLOT_Y * scale)) * 9;

	if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) && mCount[slot] != 0) {
		scene->mMouse.item = mItems[slot];
		scene->mMouse.count = mCount[slot];

		mItems[slot] = Components::Item::AIR;
		mCount[slot] = 0;

		scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
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
	float x, y;

	if (dimensions.x() <= dimensions.y()) {
		x = dimensions.x() / 4 * 3;
		y = x / INVENTORY_TEXTURE_WIDTH * INVENTORY_TEXTURE_HEIGHT;
	} else {
		y = dimensions.y() / 4 * 3;
		x = y / INVENTORY_TEXTURE_HEIGHT * INVENTORY_TEXTURE_WIDTH;
	}

	shader->set("size"_u, x / Inventory::INVENTORY_INV_SCALE, y / Inventory::INVENTORY_INV_SCALE);

	texture->activate(0);

	shader->set("offset"_u, mx - x / Inventory::INVENTORY_INV_SCALE / 2,
		    my - y / Inventory::INVENTORY_INV_SCALE / 2);

	mesh->draw(shader);
}

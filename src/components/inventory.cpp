#include "components/inventory.hpp"

#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/eventManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "screens/screen.hpp"
#include "systems/UISystem.hpp"
#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <cstddef>
#include <string>

Inventory::Inventory(class Game* game, const std::size_t size, EntityID entity)
	: Screen(game), mEntity(entity), mSize(size), mItems(size), mCount(size) {}
Inventory::Inventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Screen(game), mEntity(entity), mSize(contents[SIZE_KEY]), mItems(contents[ITEMS_KEY]),
	  mCount(contents[COUNT_KEY]) {}

nlohmann::json Inventory::save() {
	nlohmann::json contents;

	contents[SIZE_KEY] = mSize;
	contents[ITEMS_KEY] = mItems;
	contents[COUNT_KEY] = mCount;

	return contents;
}

bool Inventory::update(class Scene* scene, float) {
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

	if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) && mCount[slot] != 0 &&
	    (scene->mMouse.count == 0 || (scene->mMouse.item == mItems[slot] && scene->mMouse.count != 0))) {
		if (scene->mMouse.count != 0 && scene->mMouse.item == mItems[slot]) {
			scene->mMouse.count += mCount[slot];
		} else {
			scene->mMouse.item = mItems[slot];
			scene->mMouse.count = mCount[slot];
		}

		mItems[slot] = Components::Item::AIR;
		mCount[slot] = 0;

		scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
	}

	if (scene->mMouse.count != 0 && scene->mMouse.item != Components::Item::AIR &&
	    scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) &&
	    (mCount[slot] == 0 || mItems[slot] == scene->mMouse.item)) {
		if (mItems[slot] == scene->mMouse.item) {
			mCount[slot] += scene->mMouse.count;
		} else {
			mCount[slot] = scene->mMouse.count;
		}

		mItems[slot] = scene->mMouse.item;

		scene->mMouse.item = Components::Item::AIR;
		scene->mMouse.count = 0;

		scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
	}

	return true;
}

void Inventory::draw(class Scene* scene) {
	// Draw the inventory
	SystemManager* systemManager = mGame->getSystemManager();
	Shader* shader = systemManager->getShader("ui.vert", "ui.frag");
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	Texture* texture = systemManager->getTexture(INVENTORY_SPRITE_FILE);
	Mesh* mesh = systemManager->getUISystem()->getMesh();

	float x, y;
	float ox, oy;

	if (dimensions.x() <= dimensions.y()) {
		x = dimensions.x() / 4 * 3;
		y = x / INVENTORY_TEXTURE_WIDTH * INVENTORY_TEXTURE_HEIGHT;
		ox = x / 6;
		oy = (dimensions.y() - y) / 2;
	} else {
		y = dimensions.y() / 4 * 3;
		x = y / INVENTORY_TEXTURE_HEIGHT * INVENTORY_TEXTURE_WIDTH;
		ox = (dimensions.x() - x) / 2;
		oy = y / 6;
	}

	shader->activate();

	shader->set("texture_diffuse"_u, 0);
	shader->set("offset"_u, ox, oy);
	shader->set("size"_u, x, y);

	texture->activate(0);

	mesh->draw(shader);

	drawItems();
	drawMouse(scene);
}

void Inventory::drawItems() {
	SystemManager* systemManager = mGame->getSystemManager();
	Shader* shader = systemManager->getShader("ui.vert", "ui.frag");
	Mesh* mesh = systemManager->getUISystem()->getMesh();
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	float x, y;
	float ox, oy;
	float scale;

	if (dimensions.x() <= dimensions.y()) {
		x = dimensions.x() / 4 * 3;
		y = x / INVENTORY_TEXTURE_WIDTH * INVENTORY_TEXTURE_HEIGHT;
		ox = x / 6;
		oy = (dimensions.y() - y) / 2;

		scale = x / INVENTORY_TEXTURE_WIDTH;
	} else {
		y = dimensions.y() / 4 * 3;
		x = y / INVENTORY_TEXTURE_HEIGHT * INVENTORY_TEXTURE_WIDTH;
		ox = (dimensions.x() - x) / 2;
		oy = y / 6;

		scale = y / INVENTORY_TEXTURE_HEIGHT;
	}

	ox += INVENTORY_SLOTS_OFFSET_X * scale - (INVENTORY_SLOT_X * scale / 2 - x / INVENTORY_INV_SCALE);
	oy += INVENTORY_SLOTS_OFFSET_Y * scale - (INVENTORY_SLOT_Y * scale / 2 - y / INVENTORY_INV_SCALE);

	shader->activate();
	shader->set("texture_diffuse"_u, 0);
	shader->set("size"_u, x / INVENTORY_INV_SCALE, y / INVENTORY_INV_SCALE);

	for (std::size_t i = 0; i < mItems.size(); ++i) {
		if (mCount[i] == 0) {
			continue;
		}

		float yoff = i >= 9 ? 4 * scale : 0;

		Texture* texture = systemManager->getTexture(registers::TEXTURES.at(mItems[i]));
		texture->activate(0);

		shader->set("offset"_u, ox + i % 9 * INVENTORY_SLOT_X * scale,
			    oy + static_cast<int>(i / 9) * INVENTORY_SLOT_Y * scale + yoff);

		mesh->draw(shader);

		if (mCount[i] > 1) {
			mGame->getSystemManager()->getTextSystem()->draw(
				std::to_string(mCount[i]),
				Eigen::Vector2f(ox + i % 9 * INVENTORY_SLOT_X * scale + INVENTORY_SLOT_X / 2 * scale -
							2,
						oy + static_cast<int>(i / 9) * INVENTORY_SLOT_Y * scale - 5 + yoff),
				false);
		}

		shader->activate();
	}
}

void Inventory::drawMouse(Scene* scene) {
	// Draw Hand
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
	mx -= x / Inventory::INVENTORY_INV_SCALE / 2;
	my -= y / Inventory::INVENTORY_INV_SCALE / 2;
	shader->set("offset"_u, mx, my);

	mesh->draw(shader);

	if (scene->mMouse.count > 1) {
		mGame->getSystemManager()->getTextSystem()->draw(
			std::to_string(scene->mMouse.count),
			Eigen::Vector2f(mx + x / Inventory::INVENTORY_INV_SCALE - 2, my - 5), false);
	}
}

void Inventory::close() { mGame->getSystemManager()->getUISystem()->pop(); }

bool Inventory::tryPick(Scene* scene, const EntityID item) {
	for (std::size_t i = 0; i < mItems.size(); ++i) {
		if (mCount[i] == 0 || mItems[i] == scene->get<Components::item>(item).mType) {
			pickUp(scene, item, i);

			return true;
		}
	}

	return false;
}

// Pick da item up to the slot n
void Inventory::pickUp(Scene* scene, const EntityID item, const std::size_t index) {
	SDL_assert((mCount[index] == 0 || mItems[index] == scene->get<Components::item>(item).mType));

	mItems[index] = scene->get<Components::item>(item).mType;
	mCount[index]++;
}

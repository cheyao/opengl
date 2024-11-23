#include "components/inventory.hpp"

#include "game.hpp"
#include "managers/entityManager.hpp"
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
#include <SDL3/SDL_assert.h>
#include <cstddef>

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

void Inventory::update(class Scene*, float) {
	// Handle mouse and keys
	const auto keystate = mGame->getKeystate();

	// Quit menu
	if (keystate[SDL_SCANCODE_ESCAPE]) {
		mGame->getSystemManager()->getUISystem()->pop();
	}
}

void Inventory::draw(class Scene*) {
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

	shader->activate();
	shader->set("texture_diffuse"_u, 0);
	shader->set("size"_u, x / INVENTORY_INV_SCALE, y / INVENTORY_INV_SCALE);

	// ox, oy = Bot left
	ox += INVENTORY_SLOTS_OFFSET_X * scale - (INVENTORY_SLOT_X * scale / 2 - x / INVENTORY_INV_SCALE);
	oy += INVENTORY_SLOTS_OFFSET_Y * scale - (INVENTORY_SLOT_Y * scale / 2 - y / INVENTORY_INV_SCALE);

	for (std::size_t i = 0; i < mItems.size(); ++i) {
		if (mCount[i] == 0) {
			continue;
		}

		Texture* texture = systemManager->getTexture(registers::TEXTURES.at(mItems[i]));
		texture->activate(0);

		shader->set("offset"_u, ox + i % 9 * INVENTORY_SLOT_X * scale,
			    oy + static_cast<int>(i / 9) * INVENTORY_SLOT_Y * scale);

		mesh->draw(shader);
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

#include "components/furnace.hpp"

#include "game.hpp"
#include "managers/eventManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>

// Crafting table
FurnaceInventory::FurnaceInventory(struct furnace_t)
	: Inventory(Eigen::Vector2f(8, 8), "ui/furnace.png"), mSmeltingItems(2, Components::AIR()),
	  mSmeltingCount(2, 0), mLastCraft(0) {
	mCountRegister[getID<FurnaceInventory>()] = &mSmeltingCount;
	mItemRegister[getID<FurnaceInventory>()] = &mSmeltingItems;
}

bool FurnaceInventory::update(class Scene* const, const float) { return true; }

void FurnaceInventory::craft() {}

bool FurnaceInventory::checkRecipie(const std::uint64_t r) { return r; }

void FurnaceInventory::draw(class Scene* scene) {
	Inventory::drawInventory(scene);
	Inventory::drawItems(scene);

	SystemManager* const systemManager = mGame->getSystemManager();
	const Eigen::Vector2f dimensions = systemManager->getDemensions();
	Shader* const shader = systemManager->getShader("ui.vert", "ui.frag");
	Mesh* const mesh = systemManager->getUISystem()->getMesh();

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

	ox += (INVENTORY_SLOTS_OFFSET_X + mAX) * scale - (INVENTORY_SLOT_X * scale - sx / INVENTORY_INV_SCALE);
	oy += (INVENTORY_SLOTS_OFFSET_Y + mAY) * scale - (INVENTORY_SLOT_Y * scale - sy / INVENTORY_INV_SCALE);

	shader->activate();
	shader->set("texture_diffuse"_u, 0);
	shader->set("size"_u, sx / INVENTORY_INV_SCALE, sy / INVENTORY_INV_SCALE);

	const bool virtItems =
		scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL) || scene->getSignal(EventManager::LEFT_HOLD_SIGNAL);

	std::uint64_t vcount = 0;
	if (scene->getSignal(EventManager::LEFT_HOLD_SIGNAL)) {
		if (!mPath.empty()) {
			vcount = scene->mMouse.count / mPath.size();
		}
	} else {
		vcount = 1;
	}

	// Fuel spot
	if (!(mSmeltingCount[0] == 0 && !virtItems)) {
		auto type = mSmeltingItems[0];
		auto count = mSmeltingCount[0];
		if (virtItems) {
			if (auto s = std::ranges::find(mPath, std::make_pair(getID<FurnaceInventory>(), 0));
			    s != mPath.end()) {
				count += vcount;
				type = scene->mMouse.item;
			} else if (count == 0) {
				// This slot isn't in our path
				goto e1;
			}
		}

		Texture* const texture = systemManager->getTexture(registers::TEXTURES.at(type));
		texture->activate(0);

		shader->set("offset"_u, ox + 5, oy);

		mesh->draw(shader);

		if (count > 1) {
			mGame->getSystemManager()->getTextSystem()->draw(
				std::to_string(count), Eigen::Vector2f(ox + INVENTORY_SLOT_X / 2 * scale - 2, oy - 5),
				false);
		}

		shader->activate();
	}
e1:

	ox += mBX * scale;
	oy += mBY * scale;
	// Item spot
	if (!(mSmeltingCount[1] == 0 && !virtItems)) {
		auto type = mSmeltingItems[1];
		auto count = mSmeltingCount[1];
		if (virtItems) {
			if (auto s = std::ranges::find(mPath, std::make_pair(getID<FurnaceInventory>(), 1));
			    s != mPath.end()) {
				count += vcount;
				type = scene->mMouse.item;
			} else if (count == 0) {
				// This slot isn't in our path
				goto e2;
			}
		}

		Texture* const texture = systemManager->getTexture(registers::TEXTURES.at(type));
		texture->activate(0);

		shader->set("offset"_u, ox + 5, oy);

		mesh->draw(shader);

		if (count > 1) {
			mGame->getSystemManager()->getTextSystem()->draw(
				std::to_string(count), Eigen::Vector2f(ox + INVENTORY_SLOT_X / 2 * scale - 2, oy - 5),
				false);
		}

		shader->activate();
	}
e2:

	// Draw output slot
	// 57x9
	ox += mOutX * scale;
	oy += mOutY * scale;

	Inventory::drawMouse(scene);
}

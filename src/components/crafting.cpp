#include "components/crafting.hpp"

#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/eventManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"
#include "third_party/rapidjson/fwd.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>

// TODO: Save crafting table
CraftingInventory::CraftingInventory(class Game* game, std::uint64_t size, EntityID entity, std::uint64_t row,
				     std::uint64_t col)
	: Inventory(game, size, entity), mRows(row), mCols(col), mCraftingItems(row * col, Components::Item::AIR),
	  mCraftingCount(row * col, 0), mLastCraft(0) {}

CraftingInventory::CraftingInventory(class Game* game, const rapidjson::Value& contents, EntityID entity,
				     std::uint64_t row, std::uint64_t col)
	: Inventory(game, contents, entity), mRows(row), mCols(col), mCraftingItems(row * col, Components::Item::AIR),
	  mCraftingCount(row * col, 0), mLastCraft(0) {}

bool CraftingInventory::update(class Scene* const scene, const float delta) {
	SystemManager* const systemManager = mGame->getSystemManager();
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

	// BOTL of inv
	// 98x114
	ox += (INVENTORY_SLOTS_OFFSET_X + 98) * scale - (INVENTORY_SLOT_X * scale - sx / INVENTORY_INV_SCALE);
	oy += (INVENTORY_SLOTS_OFFSET_Y + 114) * scale - (INVENTORY_SLOT_Y * scale - sy / INVENTORY_INV_SCALE);

	const float slotx = INVENTORY_SLOT_X * scale;
	const float sloty = INVENTORY_SLOT_Y * scale;
	const float sizex = slotx * mCols;
	const float sizey = sloty * mRows;

	float mouseX, mouseY;
	auto buttons = SDL_GetMouseState(&mouseX, &mouseY);
	mouseY = dimensions.y() - mouseY;
	(void)buttons;

	// Not inside the grid
	const auto placeGrid = [&]() {
		if (mouseX < ox || mouseY < oy || mouseX > (ox + sizex) || mouseY > (oy + sizey)) {
			return;
		}

		// Normalize the buttons to grid cords
		int slot = static_cast<int>((mouseX - ox) / slotx) + static_cast<int>((mouseY - oy) / sloty) * mCols;

		if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) && mCraftingCount[slot] != 0 &&
		    (scene->mMouse.count == 0 ||
		     (scene->mMouse.item == mCraftingItems[slot] && scene->mMouse.count != 0))) {
			if (scene->mMouse.count != 0 && scene->mMouse.item == mCraftingItems[slot]) {
				scene->mMouse.count += mCraftingCount[slot];
			} else {
				scene->mMouse.item = mCraftingItems[slot];
				scene->mMouse.count = mCraftingCount[slot];
			}

			mCraftingItems[slot] = Components::Item::AIR;
			mCraftingCount[slot] = 0;

			scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
		}

		if (scene->mMouse.count != 0 && scene->mMouse.item != Components::Item::AIR &&
		    scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) &&
		    (mCraftingCount[slot] == 0 || mCraftingItems[slot] == scene->mMouse.item)) {
			if (mCraftingItems[slot] == scene->mMouse.item) {
				mCraftingCount[slot] += scene->mMouse.count;
			} else {
				mCraftingCount[slot] = scene->mMouse.count;
			}

			mCraftingItems[slot] = scene->mMouse.item;

			scene->mMouse.item = Components::Item::AIR;
			scene->mMouse.count = 0;

			scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
		}
	};

	const auto getOutput = [&]() {
		if (!scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL)) {
			return;
		}

		if (mLastCraft == 0) {
			return;
		}

		if (mouseX < ox || mouseY < oy || mouseX > (ox + slotx) || mouseY > (oy + sloty)) {
			return;
		}

		if (scene->mMouse.count != 0) {
			SDL_assert(scene->mMouse.item != Components::Item::AIR);

			return;
		}

		scene->mMouse.count = std::get<2>(registers::CRAFTING_RECIPIES.at(mLastCraft)).first;
		scene->mMouse.item = std::get<2>(registers::CRAFTING_RECIPIES.at(mLastCraft)).second;

		mLastCraft = 0;

		for (std::size_t i = 0; i < mCraftingCount.size(); ++i) {
			if (mCraftingCount[i] == 0) {
				continue;
			}

			if (mCraftingCount[i] == 1) {
				mCraftingItems[i] = Components::Item::AIR;
			}

			--mCraftingCount[i];
		}

			scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
	};

	placeGrid();

	ox += 57 * scale;
	oy += 9 * scale;

	craft();
	getOutput();

	Inventory::update(scene, delta);

	return true;
}

void CraftingInventory::craft() {
	if (mLastCraft != 0 && checkRecipie(mLastCraft)) {
		return;
	}

	bool empty = true;
	for (const auto& s : mCraftingCount) {
		if (s != 0) {
			empty = false;
			break;
		}
	}
	if (empty) {
		mLastCraft = 0;

		return;
	}

	for (std::size_t i = 1; i < registers::CRAFTING_RECIPIES.size(); ++i) {
		if (checkRecipie(i)) {
			mLastCraft = i;

			return;
		}
	}
}

bool CraftingInventory::checkRecipie(std::uint64_t r) {
	if (std::get<0>(registers::CRAFTING_RECIPIES[r]) == 0) {
		// Shapeless
		auto items = std::get<1>(registers::CRAFTING_RECIPIES[r]);

		std::uint64_t toFind = items.size();

		for (const auto& [item, count] : std::views::zip(mCraftingItems, mCraftingCount)) {
			if (item == Components::Item::AIR) {
				SDL_assert(count == 0);

				continue;
			}

			if (auto i = std::ranges::find(items, item); i != items.end()) {
				std::swap(*i, items.back());

				items.pop_back();

				--toFind;
			} else {
				return false;
			}
		}

		if (toFind == 0) {
			return true;
		} else {
			return false;
		}
	}

	// TODO: Shaped crafting

	return false;
}

void CraftingInventory::draw(class Scene* scene) {
	Inventory::draw(scene);

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

	// BOTL of inv
	ox += (INVENTORY_SLOTS_OFFSET_X + 98) * scale - (INVENTORY_SLOT_X * scale - sx / INVENTORY_INV_SCALE);
	oy += (INVENTORY_SLOTS_OFFSET_Y + 114) * scale - (INVENTORY_SLOT_Y * scale - sy / INVENTORY_INV_SCALE);

	const float slotx = INVENTORY_SLOT_X * scale;
	const float sloty = INVENTORY_SLOT_Y * scale;

	shader->activate();
	shader->set("texture_diffuse"_u, 0);
	shader->set("size"_u, sx / INVENTORY_INV_SCALE, sy / INVENTORY_INV_SCALE);

	for (std::size_t i = 0; i < mCraftingItems.size(); ++i) {
		if (mCraftingCount[i] == 0) {
			continue;
		}

		float yoff = i >= 9 ? 4 * scale : 0;

		Texture* texture = systemManager->getTexture(registers::TEXTURES.at(mCraftingItems[i]));
		texture->activate(0);

		shader->set("offset"_u, ox + i % mCols * slotx + 5, oy + static_cast<int>(i / mCols) * sloty + yoff);

		mesh->draw(shader);

		if (mCraftingCount[i] > 1) {
			mGame->getSystemManager()->getTextSystem()->draw(
				std::to_string(mCraftingCount[i]),
				Eigen::Vector2f(ox + i % mCols * slotx + INVENTORY_SLOT_X / 2 * scale - 2,
						oy + static_cast<int>(i / mCols) * sloty - 5 + yoff),
				false);
		}

		shader->activate();
	}

	// Draw output slot
	// 57x9
	ox += 57 * scale;
	oy += 9 * scale;

	if (mLastCraft == 0) {
		return;
	}

	const auto& out = std::get<2>(registers::CRAFTING_RECIPIES[mLastCraft]);
	Texture* texture = systemManager->getTexture(registers::TEXTURES.at(out.second));
	texture->activate(0);

	shader->set("offset"_u, ox + 5, oy);

	mesh->draw(shader);

	if (out.first > 1) {
		mGame->getSystemManager()->getTextSystem()->draw(
			std::to_string(out.first), Eigen::Vector2f(ox + INVENTORY_SLOT_X / 2 * scale - 2, oy - 5),
			false);
	}

	shader->activate();
}

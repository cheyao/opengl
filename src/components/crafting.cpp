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
#include "third_party/rapidjson/rapidjson.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>

// TODO: Save crafting table
CraftingInventory::CraftingInventory(class Game* game, std::uint64_t size, EntityID entity, std::uint64_t row,
				     std::uint64_t col)
	: Inventory(game, size, entity), mRows(row), mCols(col), mCraftingItems(row * col, Components::AIR()),
	  mCraftingCount(row * col, 0), mLastCraft(0) {
	mCountRegister[getID<CraftingInventory>()] = &mCraftingCount;
	mItemRegister[getID<CraftingInventory>()] = &mCraftingItems;
}

CraftingInventory::CraftingInventory(class Game* game, const rapidjson::Value& contents, EntityID entity,
				     std::uint64_t row, std::uint64_t col)
	: Inventory(game, contents, entity), mRows(row), mCols(col), mCraftingItems(), mCraftingCount(), mLastCraft(0) {
	mCountRegister[getID<CraftingInventory>()] = &mCraftingCount;
	mItemRegister[getID<CraftingInventory>()] = &mCraftingItems;

	SDL_assert(contents[CRAFTING_KEY][ITEMS_KEY].Size() == contents[CRAFTING_KEY][COUNT_KEY].Size());
	for (rapidjson::SizeType i = 0; i < contents[CRAFTING_KEY][ITEMS_KEY].Size(); i++) {
		SDL_assert(contents[CRAFTING_KEY][ITEMS_KEY][i].IsUint64());
		SDL_assert(contents[CRAFTING_KEY][COUNT_KEY][i].IsUint64());

		mCraftingItems.emplace_back(
			static_cast<Components::Item>(contents[CRAFTING_KEY][ITEMS_KEY][i].GetUint64()));
		mCraftingCount.emplace_back(contents[CRAFTING_KEY][COUNT_KEY][i].GetUint64());
	}
}

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
	SDL_GetMouseState(&mouseX, &mouseY);
	mouseY = dimensions.y() - mouseY;

	// Test if player is placing inside grid
	const auto placeGrid = [&]() {
		if (mouseX < ox || mouseY < oy || mouseX > (ox + sizex) || mouseY > (oy + sizey)) {
			return;
		}

		const int slot =
			static_cast<int>((mouseX - ox) / slotx) + static_cast<int>((mouseY - oy) / sloty) * mCols;
		if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL)) {
			// Normalize the buttons to grid cords
			if (scene->mMouse.count == 0 || mCraftingCount[slot] == 0 ||
			    (mCraftingItems[slot] != scene->mMouse.item)) {
				std::swap(scene->mMouse.count, mCraftingCount[slot]);
				std::swap(scene->mMouse.item, mCraftingItems[slot]);
			} else if (mCraftingItems[slot] == scene->mMouse.item) {
				mCraftingCount[slot] += scene->mMouse.count;
				mCraftingItems[slot] = scene->mMouse.item;

				scene->mMouse.item = Components::AIR();
				scene->mMouse.count = 0;
			}

			scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
		} else if (scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) && mPath.empty()) {
			// Not empty hand on empty slot
			if (scene->mMouse.count == 0 && mCraftingCount[slot] == 0) {
				return;
			}

			// 3 types of actions
			// One of the slot are empty: place half
			if (scene->mMouse.count == 0 && mCraftingCount[slot] != 0) {
				const auto half =
					(scene->mMouse.count ? scene->mMouse.count : mCraftingCount[slot]) / 2;
				const auto round =
					(scene->mMouse.count ? scene->mMouse.count : mCraftingCount[slot]) % 2;
				const auto item = scene->mMouse.item != Components::AIR() ? scene->mMouse.item
											  : mCraftingItems[slot];

				scene->mMouse.item = mCraftingItems[slot] = item;
				scene->mMouse.count = mCraftingCount[slot] = half;
				scene->mMouse.count += round;
				// Same block: add one to stack
			} else if (mCraftingCount[slot] == 0 || mCraftingItems[slot] == scene->mMouse.item) {
				mCraftingCount[slot] += 1;
				mCraftingItems[slot] = scene->mMouse.item;

				scene->mMouse.count -= 1;
				if (scene->mMouse.count == 0) {
					scene->mMouse.item = Components::AIR();
				}
				// different blocks in slot & hand
			} else if (mCraftingItems[slot] != scene->mMouse.item) {
				std::swap(scene->mMouse.count, mCraftingCount[slot]);
				std::swap(scene->mMouse.item, mCraftingItems[slot]);
			}

			scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;
		}
	};

	// Test if player is getting the output
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

		if (scene->mMouse.count != 0 &&
		    scene->mMouse.item != std::get<2>(registers::CRAFTING_RECIPIES.at(mLastCraft)).second) {
			SDL_assert(scene->mMouse.item != Components::AIR());

			return;
		}

		scene->mMouse.count += std::get<2>(registers::CRAFTING_RECIPIES.at(mLastCraft)).first;
		scene->mMouse.item = std::get<2>(registers::CRAFTING_RECIPIES.at(mLastCraft)).second;

		mLastCraft = 0;

		for (std::size_t i = 0; i < mCraftingCount.size(); ++i) {
			if (mCraftingCount[i] == 0) {
				continue;
			}

			if (mCraftingCount[i] == 1) {
				mCraftingItems[i] = Components::AIR();
			}

			--mCraftingCount[i];
		}

		scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
	};

	placeGrid();

	const int slot = static_cast<int>((mouseX - ox) / slotx) + static_cast<int>((mouseY - oy) / sloty) * mCols;
	if (!(mouseX < ox || mouseY < oy || mouseX > (ox + sizex) || mouseY > (oy + sizey))) {
		if (scene->getSignal(EventManager::LEFT_HOLD_SIGNAL) ||
		    scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL)) {
			// This is long click
			// Now note the slots
			if (scene->mMouse.count != 0 &&
			    (mCraftingCount[slot] == 0 || mCraftingItems[slot] == scene->mMouse.item)) {
				typename decltype(mPath)::value_type pair = {getID<CraftingInventory>(), slot};

				if (std::ranges::find(mPath, pair) == std::end(mPath)) {
					mPath.emplace_back(pair);
				}
			}
		}
	}

	// Offset to output grid
	ox += 57 * scale;
	oy += 9 * scale;

	craft();
	getOutput();

	// Finally get the inventory to update
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

	mLastCraft = 0;
}

bool CraftingInventory::checkRecipie(const std::uint64_t r) {
	const auto& recipie = registers::CRAFTING_RECIPIES[r];

	if (std::get<0>(recipie) == std::make_pair(0, 0)) {
		// Shapeless
		auto items = std::get<1>(recipie);

		std::uint64_t toFind = items.size();

		for (const auto& [item, count] : std::views::zip(mCraftingItems, mCraftingCount)) {
			// The current cell is empty
			if (item == Components::AIR()) {
				SDL_assert(count == 0);

				continue;
			}

			if (auto i = std::ranges::find(items, item); i != items.end()) {
				// Found this item, swap and pop
				std::swap(*i, items.back());
				items.pop_back();

				--toFind;
			} else {
				return false;
			}
		}

		// The recipie is valid if there isn't any more stuff to find
		if (toFind == 0) {
			return true;
		} else {
			return false;
		}
	} else {
		const auto& shape = std::get<0>(recipie);
		const auto& items = std::get<1>(recipie);

		SDL_assert(items.size() == shape.first * shape.second);

		// Recipie too big for us
		if (shape.first > mCols || shape.second > mRows) {
			return false;
		}

		for (std::size_t xoff = 0; xoff <= (mCols - shape.first); ++xoff) {
			for (std::size_t yoff = 0; yoff <= (mRows - shape.second); ++yoff) {
				for (std::size_t x = 0; x < shape.first; ++x) {
					for (std::size_t y = 0; y < shape.second; ++y) {
						if (items[x + y * shape.first] == Components::AIR()) {
							continue;
						}

						if (items[x + y * shape.first] !=
						    mCraftingItems[(x + xoff) + (y + yoff) * mCols]) {
							goto breakinnerloop;
						}
					}
				}

				return true;

			breakinnerloop:
			}
		}
	}

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

	shader->set("offset"_u, ox + 3, oy);

	mesh->draw(shader);

	if (out.first > 1) {
		mGame->getSystemManager()->getTextSystem()->draw(
			std::to_string(out.first), Eigen::Vector2f(ox + INVENTORY_SLOT_X / 2 * scale - 2, oy - 5),
			false);
	}

	shader->activate();
}

void CraftingInventory::save(rapidjson::Value& contents, rapidjson::Document::AllocatorType& allocator) {
	Inventory::save(contents, allocator);

	contents.AddMember(rapidjson::StringRef(CRAFTING_KEY), rapidjson::Value(rapidjson::kObjectType).Move(),
			   allocator);

	rapidjson::Value items(rapidjson::kArrayType);
	rapidjson::Value count(rapidjson::kArrayType);

	for (std::size_t i = 0; i < mCraftingItems.size(); ++i) {
		items.PushBack(static_cast<std::uint64_t>(mCraftingItems[i]), allocator);
		count.PushBack(mCraftingCount[i], allocator);
	}

	contents[CRAFTING_KEY].AddMember(rapidjson::StringRef(ITEMS_KEY), std::move(items.Move()), allocator);
	contents[CRAFTING_KEY].AddMember(rapidjson::StringRef(COUNT_KEY), std::move(count.Move()), allocator);
}

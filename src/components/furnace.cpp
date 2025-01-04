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
	: Inventory(Eigen::Vector2f(8, 8), "ui/furnace.png"), mSmeltingItems(3, Components::AIR()),
	  mSmeltingCount(3, 0), mFuelLeft(0), mRecipieTime(0), mLastCraft(Components::AIR()) {
	mCountRegister[getID<FurnaceInventory>()] = &mSmeltingCount;
	mItemRegister[getID<FurnaceInventory>()] = &mSmeltingItems;
}

bool FurnaceInventory::update(class Scene* const scene, const float delta) {
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
	ox += (INVENTORY_SLOTS_OFFSET_X + mFuelOffsetX) * scale - (INVENTORY_SLOT_X * scale - sx / INVENTORY_INV_SCALE);
	oy += (INVENTORY_SLOTS_OFFSET_Y + mFuelOffsetY) * scale - (INVENTORY_SLOT_Y * scale - sy / INVENTORY_INV_SCALE);

	const float slotx = INVENTORY_SLOT_X * scale;
	const float sloty = INVENTORY_SLOT_Y * scale;

	float mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	mouseY = dimensions.y() - mouseY;

	// Test if player is placing inside grid
	const auto placeGrid = [&mouseX, &mouseY, &slotx, &sloty, &scene, this](float ox, float oy, int slot) {
		if (mouseX < ox || mouseY < oy || mouseX > (ox + slotx) || mouseY > (oy + sloty)) {
			return;
		}

		if (scene->getSignal(EventManager::LEFT_HOLD_SIGNAL) ||
		    scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL)) {
			mLeftLongClick = scene->getSignal(EventManager::LEFT_HOLD_SIGNAL);

			// This is long click
			// Now note the slots
			if (scene->mMouse.count != 0 &&
			    (mSmeltingCount[slot] == 0 || mSmeltingItems[slot] == scene->mMouse.item)) {
				typename decltype(mPath)::value_type pair = {getID<FurnaceInventory>(), slot};

				if (std::ranges::find(mPath, pair) == std::end(mPath)) {
					mPath.emplace_back(pair);
				}
			}
		}

		static std::uint64_t lastClick;
		static std::int64_t lastClickPos;
		if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL)) {
			if ((mSmeltingCount[slot] == 0 && scene->mMouse.count != 0 && lastClickPos == slot &&
			     (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) - lastClick) < 300ul)) {
				SDL_assert(scene->mMouse.item != Components::AIR());

				// Here we get all stuff together
				for (const auto [i, inv] : mItemRegister) {
					for (std::size_t s = 0; s < inv->size(); ++s) {
						if ((*inv)[s] != scene->mMouse.item) {
							continue;
						}

						auto& count = (*mCountRegister[i])[s];
						scene->mMouse.count += count;
						count = 0;
						(*inv)[s] = Components::AIR();
					}
				}

				scene->getSignal(DOUBLE_CLICK_SIGNAL) = false;
				return;
			}
			// Normalize the buttons to grid cords
			if (scene->mMouse.count == 0 || mSmeltingCount[slot] == 0 ||
			    (mSmeltingItems[slot] != scene->mMouse.item)) {
				std::swap(scene->mMouse.count, mSmeltingCount[slot]);
				std::swap(scene->mMouse.item, mSmeltingItems[slot]);
			} else if (mSmeltingItems[slot] == scene->mMouse.item) {
				mSmeltingCount[slot] += scene->mMouse.count;
				mSmeltingItems[slot] = scene->mMouse.item;

				scene->mMouse.item = Components::AIR();
				scene->mMouse.count = 0;
			}

			lastClickPos = slot;
			lastClick = SDL_GetTicks();
			scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
		} else if (scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) && mPath.empty()) {
			// Not empty hand on empty slot
			if (scene->mMouse.count == 0 && mSmeltingCount[slot] == 0) {
				return;
			}

			// 3 types of actions
			// One of the slot are empty: place half
			if (scene->mMouse.count == 0 && mSmeltingCount[slot] != 0) {
				const auto half =
					(scene->mMouse.count ? scene->mMouse.count : mSmeltingCount[slot]) / 2;
				const auto round =
					(scene->mMouse.count ? scene->mMouse.count : mSmeltingCount[slot]) % 2;
				const auto item = scene->mMouse.item != Components::AIR() ? scene->mMouse.item
											  : mSmeltingItems[slot];

				scene->mMouse.item = mSmeltingItems[slot] = item;
				scene->mMouse.count = mSmeltingCount[slot] = half;
				scene->mMouse.count += round;
				if (scene->mMouse.count == 0) {
					scene->mMouse.item = Components::AIR();
				}

				// Same block: add one to stack
			} else if (mSmeltingCount[slot] == 0 || mSmeltingItems[slot] == scene->mMouse.item) {
				mSmeltingCount[slot] += 1;
				mSmeltingItems[slot] = scene->mMouse.item;

				scene->mMouse.count -= 1;
				if (scene->mMouse.count == 0) {
					scene->mMouse.item = Components::AIR();
				}
				// different blocks in slot & hand
			} else if (mSmeltingItems[slot] != scene->mMouse.item) {
				std::swap(scene->mMouse.count, mSmeltingCount[slot]);
				std::swap(scene->mMouse.item, mSmeltingItems[slot]);
			}

			scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;
		}
	};

	// Uhh
	placeGrid(ox, oy, 0);
	ox += mBX * scale;
	oy += mBY * scale;
	placeGrid(ox, oy, 1);
	ox += mOutX * scale;
	oy += mOutY * scale;
	placeGrid(ox, oy, 2);

	return Inventory::update(scene, delta);
}

void FurnaceInventory::tick(class Scene* const, float delta) {
	mFuelLeft -= delta;
	if (mFuelLeft < 0) {
		if (mSmeltingCount[FUEL_SLOT] != 0 && mLastCraft != Components::AIR() &&
		    mLastCraft == mSmeltingItems[COOK_SLOT] &&
		    (mSmeltingItems[OUTPUT_SLOT] == Components::AIR() ||
		     mSmeltingItems[OUTPUT_SLOT] == registers::SMELTING_RECIPIE.at(mLastCraft).second)) {
			mRecipieTime += delta;

			if (mRecipieTime > registers::SMELTING_RECIPIE.at(mLastCraft).first) {
				mRecipieTime = 0;

				// We add the stuff to the output
				mSmeltingCount[OUTPUT_SLOT]++;
				mSmeltingItems[OUTPUT_SLOT] = registers::SMELTING_RECIPIE.at(mLastCraft).second;
			}
		} else {
			mFuelLeft = 0;
			mRecipieTime = 0;
		}
	}
}

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

	ox += (INVENTORY_SLOTS_OFFSET_X + mFuelOffsetX) * scale - (INVENTORY_SLOT_X * scale - sx / INVENTORY_INV_SCALE);
	oy += (INVENTORY_SLOTS_OFFSET_Y + mFuelOffsetY) * scale - (INVENTORY_SLOT_Y * scale - sy / INVENTORY_INV_SCALE);

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

	const auto drawSlot = [&](float ox, float oy, int slot) {
		if (!(mSmeltingCount[slot] == 0 && !virtItems)) {
			auto type = mSmeltingItems[slot];
			auto count = mSmeltingCount[slot];
			if (virtItems) {
				if (auto s = std::ranges::find(mPath, std::make_pair(getID<FurnaceInventory>(), slot));
				    s != mPath.end()) {
					count += vcount;
					type = scene->mMouse.item;
				} else if (count == 0) {
					// This slot isn't in our path
					return;
				}
			}

			Texture* const texture = systemManager->getTexture(registers::TEXTURES.at(type));
			texture->activate(0);

			shader->set("offset"_u, ox + 5, oy);

			mesh->draw(shader);

			if (count > 1) {
				mGame->getSystemManager()->getTextSystem()->draw(
					std::to_string(count),
					Eigen::Vector2f(ox + INVENTORY_SLOT_X / 2 * scale - 2, oy - 5), false);
			}

			shader->activate();
		}
	};

	// Fuel spot
	drawSlot(ox, oy, 0);

	// Item spot
	ox += mBX * scale;
	oy += mBY * scale;
	drawSlot(ox, oy, 1);

	// Draw output slot
	ox += mOutX * scale;
	oy += mOutY * scale;
	drawSlot(ox, oy, 2);

	Inventory::drawMouse(scene);
}

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
	  mSmeltingCount(3, 0), mFuelTime(0), mFuelLeft(0), mRecipieTime(0), mLastRecipie(Components::AIR()) {
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

		if ((scene->getSignal(EventManager::LEFT_HOLD_SIGNAL) ||
		     scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL)) &&
		    slot != 2) {
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
				if (slot != 2 || scene->mMouse.count == 0) {
					std::swap(scene->mMouse.count, mSmeltingCount[slot]);
					std::swap(scene->mMouse.item, mSmeltingItems[slot]);
				}
			} else if (mSmeltingItems[slot] == scene->mMouse.item && slot != 2) {
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

			if (slot != 2) {
				// 3 types of actions
				// One of the slot are empty: place half
				if (scene->mMouse.count == 0 && mSmeltingCount[slot] != 0) {
					const auto half =
						(scene->mMouse.count ? scene->mMouse.count : mSmeltingCount[slot]) / 2;
					const auto round =
						(scene->mMouse.count ? scene->mMouse.count : mSmeltingCount[slot]) % 2;
					const auto item = scene->mMouse.item != Components::AIR()
								  ? scene->mMouse.item
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
					// different blocks in slot & hand: change
				} else if (mSmeltingItems[slot] != scene->mMouse.item) {
					std::swap(scene->mMouse.count, mSmeltingCount[slot]);
					std::swap(scene->mMouse.item, mSmeltingItems[slot]);
				}
			} else {
				if (mSmeltingItems[slot] != scene->mMouse.item &&
				    scene->mMouse.item == Components::AIR()) {
					std::swap(scene->mMouse.count, mSmeltingCount[slot]);
					std::swap(scene->mMouse.item, mSmeltingItems[slot]);
				}
			}

			scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;
		}

		const auto select = [this, &scene, &slot](const SDL_Scancode s, const std::int64_t n) {
			if (scene->getSignal(s) && (slot != 2 || mItems[n] == Components::AIR())) {
				std::swap(mSmeltingCount[slot], mCount[n]);
				std::swap(mSmeltingItems[slot], mItems[n]);
				scene->getSignal(s) = false;
			}
		};

		select(SDL_SCANCODE_1, 0);
		select(SDL_SCANCODE_2, 1);
		select(SDL_SCANCODE_3, 2);
		select(SDL_SCANCODE_4, 3);
		select(SDL_SCANCODE_5, 4);
		select(SDL_SCANCODE_6, 5);
		select(SDL_SCANCODE_7, 6);
		select(SDL_SCANCODE_8, 7);
		select(SDL_SCANCODE_9, 8);
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
	if (registers::SMELTING_RECIPIE.contains(mSmeltingItems[COOK_SLOT])) {
		const std::pair<double, Components::Item>& recipie =
			registers::SMELTING_RECIPIE.at(mSmeltingItems[COOK_SLOT]);
		if (mSmeltingItems[OUTPUT_SLOT] != Components::AIR() && mSmeltingItems[OUTPUT_SLOT] != recipie.second) {
			mRecipieTime = 0;

			goto processFuel;
		}

		if (mFuelLeft <= 0 &&
		    !(mSmeltingCount[FUEL_SLOT] >= 1 && registers::BURNING_TIME.contains(mSmeltingItems[FUEL_SLOT]))) {
			mFuelTime = 0;
			mFuelLeft = 0;
			mRecipieTime = 0;

			return;
		}

		// Here the recipie is valid
		if (mLastRecipie != recipie.second) {
			mRecipieTime = 0;
			mLastRecipie = recipie.second;
		}

		mRecipieTime += delta;
		// First check progress, then deduct fuel
		// I'm feeling generous
		if (mRecipieTime >= recipie.first) {
			mSmeltingCount[COOK_SLOT]--;
			if (mSmeltingCount[COOK_SLOT] == 0) {
				mSmeltingItems[COOK_SLOT] = Components::AIR();
			}

			mSmeltingItems[OUTPUT_SLOT] = recipie.second;
			mSmeltingCount[OUTPUT_SLOT]++;
			mRecipieTime = 0;
		}
	} else {
		mRecipieTime = 0;
	}

processFuel:
	mFuelLeft -= delta;

	if (mFuelLeft < 0) {
		// Oh no! No more fuel, get some more or abort
		if (mSmeltingCount[FUEL_SLOT] >= 1 && registers::BURNING_TIME.contains(mSmeltingItems[FUEL_SLOT]) &&
		    registers::SMELTING_RECIPIE.contains(mSmeltingItems[COOK_SLOT]) &&
		    (mSmeltingItems[OUTPUT_SLOT] == Components::AIR() ||
		     mSmeltingItems[OUTPUT_SLOT] == registers::SMELTING_RECIPIE.at(mSmeltingItems[COOK_SLOT]).second)) {
			mFuelLeft = mFuelTime = registers::BURNING_TIME.at(mSmeltingItems[FUEL_SLOT]);

			mSmeltingCount[FUEL_SLOT]--;
			if (mSmeltingCount[FUEL_SLOT] == 0) {
				mSmeltingItems[FUEL_SLOT] = Components::AIR();
			}
		} else {
			mFuelTime = 0;
			mFuelLeft = 0;
			mRecipieTime = 0;

			return;
		}
	}
}

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

	// Now draw the process & stuff

	ox -= mOutX * scale;
	oy -= mOutY * scale;
	oy -= (INVENTORY_SLOT_Y + 1) * scale;
	ox -= 1 * scale;

	// 1. Fuel fire
	if (mFuelLeft > 0 && mFuelTime != 0) {
		const float percentage = mFuelLeft / mFuelTime;
		Texture* const fireTexture = mGame->getSystemManager()->getTexture("ui/lit-progress.png");
		Shader* const percentShader = mGame->getSystemManager()->getShader("percentage.vert", "ui.frag");
		const Eigen::Vector2f size = fireTexture->getSize() / 7 * scale;

		percentShader->activate();
		percentShader->set("texture_diffuse"_u, 0);
		percentShader->set("size"_u, size);
		percentShader->set("offset"_u, ox, oy);
		percentShader->set("percent"_u, percentage);
		percentShader->set("vertical"_u, true);
		fireTexture->activate(0);

		mesh->draw(percentShader);
	}

	ox += 23 * scale;

	// 2. Recipie time left
	if (mRecipieTime > 0) {
		const float percentage = mRecipieTime / registers::SMELTING_RECIPIE.at(mSmeltingItems[COOK_SLOT]).first;
		Texture* const progressTexture = mGame->getSystemManager()->getTexture("ui/burn-progress.png");
		Shader* const percentShader = mGame->getSystemManager()->getShader("percentage.vert", "ui.frag");
		const Eigen::Vector2f size = progressTexture->getSize() / 7 * scale;

		percentShader->activate();
		percentShader->set("texture_diffuse"_u, 0);
		percentShader->set("size"_u, size);
		percentShader->set("offset"_u, ox, oy);
		percentShader->set("percent"_u, percentage);
		percentShader->set("vertical"_u, false);
		progressTexture->activate(0);

		mesh->draw(percentShader);
	}

	shader->activate();

	Inventory::drawMouse(scene);
}

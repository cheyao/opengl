#include "components/inventory.hpp"

#include "game.hpp"
#include "items.hpp"
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
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/rapidjson.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_timer.h>
#include <cstddef>
#include <cstdint>
#include <string>

std::vector<Components::Item> Inventory::mItems;
std::vector<std::uint64_t> Inventory::mCount;

Inventory::Inventory(class Game*, const std::size_t size)
	: Screen(Game::getInstance()), mSize(size), mLeftLongClick(0), mCounter(0) {
	mItems = std::vector<Components::Item>(size);
	mCount = std::vector<std::uint64_t>(size);

	mCountRegister[getID<Inventory>()] = &mCount;
	mItemRegister[getID<Inventory>()] = &mItems;
}

Inventory::Inventory(class Game* game, const rapidjson::Value& contents) : Screen(game) {
	mItems = std::vector<Components::Item>();
	mCount = std::vector<std::uint64_t>();

	mCountRegister[getID<Inventory>()] = &mCount;
	mItemRegister[getID<Inventory>()] = &mItems;

	SDL_assert(contents[SIZE_KEY].IsUint64());
	mSize = contents[SIZE_KEY].GetUint64();

	SDL_assert(contents[ITEMS_KEY].Size() == contents[COUNT_KEY].Size());
	for (rapidjson::SizeType i = 0; i < contents[ITEMS_KEY].Size(); i++) {
		SDL_assert(contents[ITEMS_KEY][i].IsUint64());
		SDL_assert(contents[COUNT_KEY][i].IsUint64());

		mItems.emplace_back(static_cast<Components::Item>(contents[ITEMS_KEY][i].GetUint64()));
		mCount.emplace_back(contents[COUNT_KEY][i].GetUint64());
	}
}

// A view of the main inventory
Inventory::Inventory(const Eigen::Vector2f& offset, const std::string& texture)
	: Screen(Game::getInstance()), INVENTORY_SLOTS_OFFSET_X(offset.x()), INVENTORY_SLOTS_OFFSET_Y(offset.y()),
	  INVENTORY_SPRITE_FILE(texture) {}

void Inventory::save(rapidjson::Value& contents, rapidjson::Document::AllocatorType& allocator) {
	contents.AddMember(rapidjson::StringRef(SIZE_KEY), mSize, allocator);

	rapidjson::Value items(rapidjson::kArrayType);
	rapidjson::Value count(rapidjson::kArrayType);

	for (std::size_t i = 0; i < mItems.size(); ++i) {
		items.PushBack(static_cast<std::uint64_t>(mItems[i]), allocator);
		count.PushBack(mCount[i], allocator);
	}

	contents.AddMember(rapidjson::StringRef(ITEMS_KEY), std::move(items.Move()), allocator);
	contents.AddMember(rapidjson::StringRef(COUNT_KEY), std::move(count.Move()), allocator);
}

bool Inventory::update(class Scene* scene, float) {
	handleKeys();

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

	// Add the offset of the inv texture. and substract the size of half a slot minus something
	ox += INVENTORY_SLOTS_OFFSET_X * scale - (INVENTORY_SLOT_X * scale / 2 - sx / INVENTORY_INV_SCALE);
	oy += INVENTORY_SLOTS_OFFSET_Y * scale - (INVENTORY_SLOT_Y * scale / 2 - sy / INVENTORY_INV_SCALE);

	float mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	mouseY = dimensions.y() - mouseY;

	mouseX -= ox;
	mouseY -= oy;

	static std::uint64_t mLastClick;
	static std::int64_t mLastClickPos;
	const std::int64_t slot = static_cast<std::int64_t>(mouseX / (INVENTORY_SLOT_X * scale)) +
				  static_cast<std::int64_t>(mouseY / (INVENTORY_SLOT_Y * scale)) * 9;
	if (scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL) || scene->getSignal(EventManager::LEFT_HOLD_SIGNAL)) {
		// Here we register all the slots touched by our long click
		if (mouseX < 0 || mouseY < 0 || mouseX > (9 * INVENTORY_SLOT_X * scale) ||
		    mouseY > (4 * INVENTORY_SLOT_Y * scale)) {
			goto endLogic;
		}

		mLeftLongClick = scene->getSignal(EventManager::LEFT_HOLD_SIGNAL);

		// This is long click
		// Now note the slots
		if (scene->mMouse.count != 0 && (mCount[slot] == 0 || mItems[slot] == scene->mMouse.item)) {
			typename decltype(mPath)::value_type pair = {getID<Inventory>(), slot};

			if (std::ranges::find(mPath, pair) == std::end(mPath)) {
				mPath.emplace_back(pair);
			}
		}
	} else if ((!scene->getSignal(EventManager::LEFT_HOLD_SIGNAL) &&
		    !scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL)) &&
		   !mPath.empty()) {
		// Here we redistribute the items
		if (mLeftLongClick) {
			const auto c = scene->mMouse.count / mPath.size();
			const auto left = scene->mMouse.count - mPath.size() * c;

			for (const auto& [id, s] : mPath) {
				(*mCountRegister[id])[s] += c;
				(*mItemRegister[id])[s] = static_cast<Components::Item>(scene->mMouse.item);
			}

			scene->mMouse.count = left;
			if (scene->mMouse.count == 0) {
				scene->mMouse.item = Components::AIR();
			}
		} else {
			for (const auto& [id, s] : mPath) {
				(*mCountRegister[id])[s] += 1;
				(*mItemRegister[id])[s] = static_cast<Components::Item>(scene->mMouse.item);
				scene->mMouse.count--;

				if (scene->mMouse.count == 0) {
					scene->mMouse.item = Components::AIR();
					break;
				}
			}
		}

		mPath.clear();
	} else if (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) && mPath.empty()) {
		if (mouseX < 0 || mouseY < 0 || mouseX > (9 * INVENTORY_SLOT_X * scale) ||
		    mouseY > (4 * INVENTORY_SLOT_Y * scale)) {
			goto endLogic;
		}

		if ((mCount[slot] == 0 && scene->mMouse.count != 0 && mLastClickPos == slot &&
		     (scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) - mLastClick) < 300ul) ||
		    scene->getSignal(DOUBLE_CLICK_SIGNAL)) {
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
			goto endLogic;
		}

		// Not inside the space
		if (scene->mMouse.count == 0 || mCount[slot] == 0 || (mItems[slot] != scene->mMouse.item)) {
			std::swap(scene->mMouse.count, mCount[slot]);
			std::swap(scene->mMouse.item, mItems[slot]);
		} else if (mItems[slot] == scene->mMouse.item) {
			mCount[slot] += scene->mMouse.count;
			mItems[slot] = scene->mMouse.item;

			scene->mMouse.item = Components::AIR();
			scene->mMouse.count = 0;
		}

		mLastClickPos = slot;
		mLastClick = SDL_GetTicks();
		scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
	} else if (scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) && mPath.empty()) {
		if (mouseX < 0 || mouseY < 0 || mouseX > (9 * INVENTORY_SLOT_X * scale) ||
		    mouseY > (4 * INVENTORY_SLOT_Y * scale)) {
			goto endLogic;
		}

		// Not empty hand on empty slot
		if (scene->mMouse.count == 0 && mCount[slot] == 0) {
			goto endLogic;
		}

		// 3 types of actions
		// One of the slot are empty: place half
		if (scene->mMouse.count == 0 && mCount[slot] != 0) {
			const auto half = (scene->mMouse.count ? scene->mMouse.count : mCount[slot]) / 2;
			const auto round = (scene->mMouse.count ? scene->mMouse.count : mCount[slot]) % 2;
			const auto item = scene->mMouse.item != Components::AIR() ? scene->mMouse.item : mItems[slot];

			scene->mMouse.item = mItems[slot] = item;
			scene->mMouse.count = mCount[slot] = half;
			scene->mMouse.count += round;
			// Same block: add one to stack
		} else if (mCount[slot] == 0 || mItems[slot] == scene->mMouse.item) {
			mCount[slot] += 1;
			mItems[slot] = scene->mMouse.item;

			scene->mMouse.count -= 1;
			if (scene->mMouse.count == 0) {
				scene->mMouse.item = Components::AIR();
			}
			// different blocks in slot & hand
		} else if (mItems[slot] != scene->mMouse.item) {
			std::swap(scene->mMouse.count, mCount[slot]);
			std::swap(scene->mMouse.item, mItems[slot]);
		}

		scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;
	}
endLogic:

	return true;
}

void Inventory::handleKeys() {
	// Handle mouse and keys
	const auto keystate = mGame->getKeystate();

	// Quit menu
	if (keystate[SDL_SCANCODE_ESCAPE]) {
		mGame->getSystemManager()->getUISystem()->pop();
	}
}

void Inventory::draw(class Scene* scene) {
	drawInventory(scene);
	drawItems(scene);
	drawMouse(scene);
}

void Inventory::drawInventory(class Scene*) {
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
}

void Inventory::drawItems(class Scene* const scene) {
	SystemManager* const systemManager = mGame->getSystemManager();
	Shader* const shader = systemManager->getShader("ui.vert", "ui.frag");
	Mesh* const mesh = systemManager->getUISystem()->getMesh();
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

	for (std::size_t i = 0; i < mItems.size(); ++i) {
		if (mCount[i] == 0 && !virtItems) {
			continue;
		}

		auto type = mItems[i];
		auto count = mCount[i];
		if (virtItems) {
			if (auto s = std::ranges::find(mPath, std::make_pair(getID<Inventory>(), i));
			    s != mPath.end()) {
				count += vcount;
				type = scene->mMouse.item;
			} else if (count == 0) {
				// This slot isn't in our path
				continue;
			}
		}

		const float yoff = i >= 9 ? 4 * scale : 0;

		Texture* const texture = systemManager->getTexture(registers::TEXTURES.at(type));
		texture->activate(0);

		shader->set("offset"_u, ox + i % 9 * INVENTORY_SLOT_X * scale,
			    oy + static_cast<int>(i / 9) * INVENTORY_SLOT_Y * scale + yoff);

		mesh->draw(shader);

		if (count > 1) {
			mGame->getSystemManager()->getTextSystem()->draw(
				std::to_string(count),
				Eigen::Vector2f(ox + (i % 9 + 0.5) * INVENTORY_SLOT_X * scale - 3,
						oy + static_cast<int>(i / 9) * INVENTORY_SLOT_Y * scale - 5 + yoff),
				false);
		}

		shader->activate();
	}
}

void Inventory::drawMouse(Scene* scene) {
	// Draw Hand
	if (scene->mMouse.item == Components::AIR()) {
		return;
	}

	const bool virtItems =
		scene->getSignal(EventManager::RIGHT_HOLD_SIGNAL) || scene->getSignal(EventManager::LEFT_HOLD_SIGNAL);
	std::uint64_t vcount = 0;
	if (virtItems) {
		if (scene->getSignal(EventManager::LEFT_HOLD_SIGNAL)) {
			if (!mPath.empty()) {
				vcount = scene->mMouse.count - scene->mMouse.count % mPath.size();
			}
		} else {
			vcount = SDL_min(scene->mMouse.count, mPath.size());
		}
	}

	SystemManager* const systemManager = mGame->getSystemManager();
	Shader* const shader = systemManager->getShader("ui.vert", "ui.frag");
	Mesh* const mesh = systemManager->getUISystem()->getMesh();
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

	if ((scene->mMouse.count - vcount) > 1) {
		mGame->getSystemManager()->getTextSystem()->draw(
			std::to_string(scene->mMouse.count - vcount),
			Eigen::Vector2f(mx + x / Inventory::INVENTORY_INV_SCALE - 3, my - 5), false);
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

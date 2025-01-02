#include "components/playerInventory.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"
#include "third_party/rapidjson/fwd.h"

#include <SDL3/SDL.h>
#include <cstddef>

PlayerInventory::PlayerInventory(class Game* game, const std::size_t size)
	: CraftingInventory(game, size, 2, 2), mSelect(0) {}

PlayerInventory::PlayerInventory(class Game* game, const rapidjson::Value& contents)
	: CraftingInventory(game, contents, 2, 2), mSelect(0) {}

bool PlayerInventory::update(class Scene* scene, float delta) { return CraftingInventory::update(scene, delta); }

void PlayerInventory::draw(class Scene* scene) { CraftingInventory::draw(scene); }

void PlayerInventory::tryPlace(class Scene* scene, const Eigen::Vector2i& pos) {
	if (mItems[mSelect] == Components::AIR() || mCount[mSelect] == 0) {
		return;
	}

	// Don't place if the block isn't placable
	if (!registers::BREAK_TIMES.contains(mItems[mSelect])) {
		return;
	}

	Texture* texture = mGame->getSystemManager()->getTexture(registers::TEXTURES.at(mItems[mSelect]));
	const EntityID entity = scene->newEntity();
	scene->emplace<Components::block>(entity, mItems[mSelect], pos);
	scene->emplace<Components::texture>(entity, texture);
	scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(), true);

	--mCount[mSelect];
	if (mCount[mSelect] == 0) {
		mItems[mSelect] = Components::AIR();
	}
}

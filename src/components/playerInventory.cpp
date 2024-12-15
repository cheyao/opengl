#include "components/playerInventory.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <cstddef>

PlayerInventory::PlayerInventory(class Game* game, std::size_t size, EntityID entity)
	: Inventory(game, size, entity), mSelect(0) {}
PlayerInventory::PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Inventory(game, contents, entity), mSelect(0) {}

void PlayerInventory::tryPlace(class Scene* scene, const Eigen::Vector2i& pos) {
	if (mItems[mSelect] == Components::Item::AIR || mCount[mSelect] == 0) {
		return;
	}

	Texture* texture = mGame->getSystemManager()->getTexture(registers::TEXTURES.at(mItems[mSelect]));
	const EntityID entity = scene->newEntity();
	scene->emplace<Components::block>(entity, mItems[mSelect], pos);
	scene->emplace<Components::texture>(entity, texture);
	scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(), true);

	--mCount[mSelect];
	if (mCount[mSelect] == 0) {
		mItems[mSelect] = Components::Item::AIR;
	}
}

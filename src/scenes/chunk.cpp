#include "scenes/chunk.hpp"

class Texture;

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <vector>

Chunk::Chunk(Game* game, Scene* scene, const std::int64_t position) : mPosition(position) {
	mBlocks.reserve(32);
	for (size_t i = 0; i < 32; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	// Spawn blocks
	for (auto i = 0; i < 16; ++i) {
		mBlocks[i].emplace_back(scene->newEntity());
		const EntityID& entity = mBlocks[i].back();
		scene->emplace<Components::block>(entity, Components::block::STONE);
		scene->emplace<Components::texture>(entity, game->getSystemManager()->getTexture("stone.png", true));
		scene->emplace<Components::position>(entity, Eigen::Vector2f(400.0f + i * game->getSystemManager()->getTexture("stone.png", true)->getWidth(), 10.0f));
		scene->emplace<Components::collision>(
			entity, Eigen::Vector2f(0.0f, 0.0f),
			Eigen::Vector2f(game->getSystemManager()->getTexture("stone.png", true)->getWidth(),
					game->getSystemManager()->getTexture("stone.png", true)->getHeight()),
			true);
	}
}

Chunk::Chunk(Game* game, Scene* scene, nlohmann::json data) : mPosition(data["position"]) {
	mBlocks.reserve(32);
	for (size_t i = 0; i < 32; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	(void)game;
	(void)scene;
}

Chunk::~Chunk() {}

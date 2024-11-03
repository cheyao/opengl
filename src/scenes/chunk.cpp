#include "scenes/chunk.hpp"

class Texture;

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <functional>
#include <vector>

// TODO: Backup
Chunk::Chunk(Game* game, Scene* scene, const std::int64_t position) : mPosition(position) {
	mBlocks.reserve(CHUNK_WIDTH);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	Texture* stone = game->getSystemManager()->getTexture("stone.png", true);

	// Spawn blocks
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks[i].emplace_back(scene->newEntity());
		const EntityID& entity = mBlocks[i].back();
		scene->emplace<Components::block>(entity, Components::block::STONE,
						  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, 0));
		scene->emplace<Components::texture>(entity, stone);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), stone->getSize(), true);
	}
	Texture* grass = game->getSystemManager()->getTexture("grass-block.png", true);

	// The second layer of grass
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks[i].emplace_back(scene->newEntity());
		const EntityID& entity = mBlocks[i].back();
		scene->emplace<Components::block>(entity, Components::block::GRASS_BLOCK,
						  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, 1));
		scene->emplace<Components::texture>(entity, grass);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), grass->getSize(), true);
	}
}

Chunk::Chunk(Game* game, Scene* scene, const nlohmann::json& data) : mPosition(data["position"]) {
	mBlocks.reserve(CHUNK_WIDTH);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	auto getTexture = std::bind(&SystemManager::getTexture, game->getSystemManager(), std::placeholders::_1,
				    std::placeholders::_2);

	// Load the things only when we need them
	const static std::unordered_map<Components::block::BlockType, std::function<Texture*()>> blockToTexture = {
		{Components::block::STONE, [&getTexture] { return getTexture("stone.png", true); }},
		{Components::block::GRASS_BLOCK, [&getTexture] { return getTexture("grass-block.png", true); }},
	};

	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		for (std::size_t j = 0; j < data["blocks"][i].size(); ++j) {
			const auto& block = static_cast<Components::block::BlockType>(data["blocks"][i][j]);

			if (block == Components::block::AIR) {
				mBlocks[i].emplace_back(0);
				continue;
			}

			SDL_assert(blockToTexture.contains(block));

			Texture* texture = blockToTexture.at(block)();

			mBlocks[i].emplace_back(scene->newEntity());

			const EntityID& entity = mBlocks[i].back();
			scene->emplace<Components::block>(entity, block,
							  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, j));
			scene->emplace<Components::texture>(entity, texture);
			scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(),
							      true);
		}
	}
}

Chunk::~Chunk() { SDL_assert(mBlocks.empty() && "Didn't save?"); }

nlohmann::json Chunk::save(Scene* scene) {
	nlohmann::json chunk;

	chunk["position"] = mPosition;
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		for (std::size_t j = 0; j < mBlocks[i].size(); ++j) {
			if (!scene->valid(mBlocks[i][j])) {
				chunk["blocks"][i][j] = Components::block::AIR;
				continue;
			}

			chunk["blocks"][i][j] =
				static_cast<std::uint64_t>(scene->get<Components::block>(mBlocks[i][j]).mType);
		}
	}

	// Only clear when assert is enabled
	SDL_assert((mBlocks.clear(), true));

	return chunk;
}

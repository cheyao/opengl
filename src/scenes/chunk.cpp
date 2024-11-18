#include "scenes/chunk.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include "managers/systemManager.hpp"
#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

// TODO: Backup
Chunk::Chunk(Game* game, Scene* scene, const std::int64_t position)
	: mPosition(position), mBlocks(CHUNK_WIDTH, std::vector<EntityID>()) {
	// NOTE: The order is important!!!
	// The save saves from the array in a lifo order
	// So the top block must go in last!

	// Spawn blocks
	Texture* stone = game->getSystemManager()->getTexture("blocks/stone.png", true);
	for (auto y = 0; y < WATER_LEVEL; ++y) {
		for (auto i = 0; i < CHUNK_WIDTH; ++i) {
			mBlocks[i].emplace_back(scene->newEntity());
			const EntityID& entity = mBlocks[i].back();
			scene->emplace<Components::block>(entity, Components::Item::STONE,
							  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, y));
			scene->emplace<Components::texture>(entity, stone);
			scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), stone->getSize(),
							      true);
		}
	}

	// The second layer of grass
	Texture* grass = game->getSystemManager()->getTexture("blocks/grass-block.png", true);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks[i].emplace_back(scene->newEntity());
		const EntityID& entity = mBlocks[i].back();
		scene->emplace<Components::block>(entity, Components::Item::GRASS_BLOCK,
						  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, WATER_LEVEL));
		scene->emplace<Components::texture>(entity, grass);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), grass->getSize(), true);
	}
}

// Loading from save
Chunk::Chunk(Game* game, Scene* scene, const nlohmann::json& data) : mPosition(data["position"]) {
	mBlocks.reserve(CHUNK_WIDTH);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	auto systemManager = game->getSystemManager();
	auto getTexture = [systemManager](const std::string& name) { return systemManager->getTexture(name, true); };

	// Load the things only when we need them
	// FIXME: Better load on demand
	const static std::unordered_map<Components::Item, std::function<Texture*()>> BLOCK_TO_TEXTURE = {
		{Components::Item::GRASS_BLOCK, [&getTexture] { return getTexture("blocks/grass-block.png"); }},
		{Components::Item::STONE, [&getTexture] { return getTexture("blocks/stone.png"); }},
	};

	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		for (std::size_t j = 0; j < data["blocks"][i].size(); ++j) {
			const auto& block = static_cast<Components::Item>(data["blocks"][i][j]);

			if (block == Components::Item::AIR) {
				mBlocks[i].emplace_back(0);
				continue;
			}

			SDL_assert(BLOCK_TO_TEXTURE.contains(block));

			Texture* texture = BLOCK_TO_TEXTURE.at(block)();

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

nlohmann::json Chunk::save(Scene* scene) {
	nlohmann::json chunk;

	// FIXME:Get blocks from scene instead of vector
	chunk["position"] = mPosition;
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		for (std::size_t j = 0; j < mBlocks[i].size(); ++j) {
			if (!scene->valid(mBlocks[i][j]) || !scene->contains<Components::block>(mBlocks[i][j])) {
				chunk["blocks"][i][j] = Components::Item::AIR;
				continue;
			}

			// Hmm
			chunk["blocks"][i][j] =
				static_cast<std::uint64_t>(scene->get<Components::block>(mBlocks[i][j]).mType);
		}
	}

	for (const auto& layer : mBlocks) {
		for (const auto& block : layer) {
			if (block != Components::Item::AIR) {
				scene->erase(block);
			}
		}
	}

	return chunk;
}

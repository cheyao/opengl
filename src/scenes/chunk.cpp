#include "scenes/chunk.hpp"

#include "components.hpp"
#include "components/noise.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

// TODO: Backup
Chunk::Chunk(Game* game, Scene* scene, const NoiseGenerator* const noise, const std::int64_t position)
	: mPosition(position) {

	// Spawn blocks
	Texture* stone = game->getSystemManager()->getTexture("blocks/stone.png", true);
	Texture* grass = game->getSystemManager()->getTexture("blocks/grass-block.png", true);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		double height = noise->getNoise(i + position * CHUNK_WIDTH);
		std::uint64_t block_height = WATER_LEVEL + 5 * height;

		for (std::uint64_t y = 0; y < block_height; ++y) {
			const EntityID entity = scene->newEntity();
			scene->emplace<Components::block>(entity, Components::Item::STONE,
							  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, y));
			scene->emplace<Components::texture>(entity, stone);
			scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), stone->getSize(),
							      true);
		}

		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, Components::Item::GRASS_BLOCK,
						  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, block_height));
		scene->emplace<Components::texture>(entity, grass);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), grass->getSize(), true);
	}
}

// Loading from save
Chunk::Chunk(const nlohmann::json& data, Game* game, Scene* scene) : mPosition(data[POSITION_KEY]) {
	if (!data.contains(BLOCKS_KEY)) {
		throw std::runtime_error("Invalid chunk");
	}

	for (const auto& it : data[BLOCKS_KEY]) {
		const Components::Item block = it[0];

		SDL_assert(registers::TEXTURES.contains(block));

		Texture* texture = game->getSystemManager()->getTexture(registers::TEXTURES.at(block), true);

		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, block, it[1].template get<Eigen::Vector2i>());
		scene->emplace<Components::texture>(entity, texture);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(), true);
	}
}

nlohmann::json Chunk::save(Scene* scene) {
	nlohmann::json chunk;
	chunk[POSITION_KEY] = mPosition;

	for (const auto item : scene->view<Components::item>()) {
		// 0 runs??
		SDL_Log("Item");
		// Not in the chunk
		if ((scene->get<Components::position>(item).mPosition.x() / Components::block::BLOCK_SIZE /
			     CHUNK_WIDTH -
		     (scene->get<Components::position>(item).mPosition.x() < 0)) != mPosition) {
			continue;
		}

		chunk[ITEMS_KEY].push_back({etoi(scene->template get<Components::item>(item).mType),
					    scene->template get<Components::position>(item).mPosition});

		scene->erase(item);
	}

	for (const auto block : scene->template view<Components::block>()) {
		// Not in the chunk
		if ((scene->template get<Components::block>(block).mPosition.x() / CHUNK_WIDTH -
		     (scene->template get<Components::block>(block).mPosition.x() < 0)) != mPosition) {
			continue;
		}

		// Here we store the block as type pos pos
		chunk[BLOCKS_KEY].push_back({etoi(scene->template get<Components::block>(block).mType),
					     scene->template get<Components::block>(block).mPosition});

		scene->erase(block);
	}

	return chunk;
}

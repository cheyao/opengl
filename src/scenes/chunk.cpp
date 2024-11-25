#include "scenes/chunk.hpp"

#include "components.hpp"
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
#include <vector>

// TODO: Backup
Chunk::Chunk(Game* game, Scene* scene, const std::int64_t position) : mPosition(position) {
	// Spawn blocks
	Texture* stone = game->getSystemManager()->getTexture("blocks/stone.png", true);
	for (auto y = 0; y < WATER_LEVEL; ++y) {
		for (auto i = 0; i < CHUNK_WIDTH; ++i) {
			const EntityID entity = scene->newEntity();
			scene->emplace<Components::block>(entity, Components::Item::STONE,
							  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, y));
			scene->emplace<Components::texture>(entity, stone);
			scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), stone->getSize(),
							      true);
		}
	}

	// The top layer of grass
	Texture* grass = game->getSystemManager()->getTexture("blocks/grass-block.png", true);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, Components::Item::GRASS_BLOCK,
						  Eigen::Vector2i(i + mPosition * CHUNK_WIDTH, WATER_LEVEL));
		scene->emplace<Components::texture>(entity, grass);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), grass->getSize(), true);
	}
}

// Loading from save
Chunk::Chunk(Game* game, Scene* scene, const nlohmann::json& data) : mPosition(data[POSITION_KEY]) {

	for (const auto& it : data[CONTENTS_KEY]) {
		const auto block = static_cast<Components::Item>(it[0]);

		SDL_assert(registers::TEXTURES.contains(block));

		Texture* texture = game->getSystemManager()->getTexture(registers::TEXTURES.at(block), true);

		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(
			entity, block,
			Eigen::Vector2i(static_cast<std::int64_t>(it[1][0]) + mPosition * CHUNK_WIDTH, it[1][1]));
		scene->emplace<Components::texture>(entity, texture);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(), true);
	}
}

nlohmann::json Chunk::save(Scene* scene) {
	nlohmann::json chunk;

	chunk[POSITION_KEY] = mPosition;
	for (const auto block : scene->view<Components::block>()) {
		// Not in the chunk
		if (scene->get<Components::block>(block).mPosition.x() / CHUNK_WIDTH -
			    (scene->get<Components::block>(block).mPosition.x() < 0) !=
		    mPosition) {
			continue;
		}

		// Here we store the block as type pos pos
		chunk[CONTENTS_KEY].push_back({static_cast<std::uint64_t>(scene->get<Components::block>(block).mType),
					       scene->get<Components::block>(block).mPosition});

		scene->erase(block);
	}

	return chunk;
}

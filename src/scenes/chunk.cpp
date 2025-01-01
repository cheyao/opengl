#include "scenes/chunk.hpp"

#include "components.hpp"
#include "components/noise.hpp"
#include "game.hpp"
#include "items.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"
#include "third_party/rapidjson/allocators.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/rapidjson.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <cstdint>

// TODO: Backup
Chunk::Chunk(Game* game, Scene* scene, const NoiseGenerator* const noise, const std::int64_t position)
	: mPosition(position), mGame(game) {

	// Spawn blocks
	Texture* stone = game->getSystemManager()->getTexture("blocks/stone.png");
	Texture* grass = game->getSystemManager()->getTexture("blocks/grass-block.png");

	const auto offset = mPosition * CHUNK_WIDTH;

	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		double height = noise->getNoise(i + position * CHUNK_WIDTH);
		std::uint64_t block_height = WATER_LEVEL + 5 * height;
		mHeightMap[i] = block_height;

		for (std::uint64_t y = 0; y < block_height; ++y) {
			const EntityID entity = scene->newEntity();
			scene->emplace<Components::block>(entity, Components::Item::STONE,
							  Eigen::Vector2i(i + offset, y));
			scene->emplace<Components::texture>(entity, stone);
			scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), stone->getSize(),
							      true);
		}

		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, Components::Item::GRASS_BLOCK,
						  Eigen::Vector2i(i + offset, block_height));
		scene->emplace<Components::texture>(entity, grass);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), grass->getSize(), true);

		// Spawn structures
		for (const auto& [chance, structure] : registers::SURFACE_STRUCTURES) {
			float roll = SDL_randf();

			// Rig the roll so there is always a tree near
			if (i + offset == 3) {
				roll = 0;
			}

			if (roll < chance) {
				spawnStructure(Eigen::Vector2i(i + offset, block_height + 1), structure, scene);
			}
		}
	}
}

// Loading from save
Chunk::Chunk(const rapidjson::Value& data, Game* game, Scene* scene)
	: mPosition(data[POSITION_KEY].GetInt64()), mGame(game) {

	for (rapidjson::SizeType i = 0; i < data[BLOCKS_KEY].Size(); i++) {
		const Components::Item block = static_cast<Components::Item>(data[BLOCKS_KEY][i][0].GetUint64());

		SDL_assert(registers::TEXTURES.contains(block));

		Texture* texture = game->getSystemManager()->getTexture(registers::TEXTURES.at(block));

		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, block, getVector2i(data[BLOCKS_KEY][i][1]));
		scene->emplace<Components::texture>(entity, texture);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(), true);
	}
}

void Chunk::save(class Scene* scene, rapidjson::Value& chunk, rapidjson::MemoryPoolAllocator<>& allocator) {
	chunk.AddMember(rapidjson::StringRef(POSITION_KEY), rapidjson::Value(mPosition).Move(), allocator);
	chunk.AddMember(rapidjson::StringRef(BLOCKS_KEY), rapidjson::Value(rapidjson::kArrayType).Move(), allocator);

	/*
	for (const auto item : scene->view<Components::item>()) {
		// 0 runs??
		SDL_Log("Item");

		// Not in the chunk
		if ((scene->get<Components::position>(item).mPosition.x() / Components::block::BLOCK_SIZE /
			     CHUNK_WIDTH -
		     (scene->get<Components::position>(item).mPosition.x() < 0)) != mPosition) {
			continue;
		}

		chunk[ITEMS_KEY].PushBack({etoi(scene->template get<Components::item>(item).mType),
					    scene->template get<Components::position>(item).mPosition});

		scene->erase(item);
	}
	*/

	for (const auto block : scene->view<Components::block>()) {
		// Not in the chunk
		if ((scene->get<Components::block>(block).mPosition.x() / CHUNK_WIDTH -
		     (scene->get<Components::block>(block).mPosition.x() < 0)) != mPosition) {
			continue;
		}

		// Here we store the block as type pos pos
		rapidjson::Value i(rapidjson::kArrayType);

		i.PushBack(etoi(scene->template get<Components::block>(block).mType), allocator);
		i.PushBack(fromVector2i(scene->template get<Components::block>(block).mPosition, allocator).Move(),
			   allocator);

		chunk[BLOCKS_KEY].PushBack(i.Move(), allocator);

		scene->erase(block);
	}
}

void Chunk::spawnStructure(const Eigen::Vector2i& pos,
			   const std::vector<std::pair<Components::Item, Eigen::Vector2i>> structure, Scene* scene) {
	const auto& blocks = scene->view<Components::block>();
	for (const auto& [blockType, offset] : structure) {
		const Eigen::Vector2i position = offset + pos;

		bool occupied = false;
		for (const auto block : blocks) {
			if (scene->get<Components::block>(block).mPosition == position) {
				occupied = true;
				break;
			}
		}

		if (occupied) {
			continue;
		}

		Texture* texture = mGame->getSystemManager()->getTexture(registers::TEXTURES.at(blockType));
		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, blockType, position);
		scene->emplace<Components::texture>(entity, texture);
		scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(), true);
	}
}

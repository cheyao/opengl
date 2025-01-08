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

Chunk::Chunk(Scene* scene, NoiseGenerator* const noise, const std::int64_t position) : mPosition(position) {
	// We shall first generate a chunk map
	// Then we shall spawn the blocks

	// Indexed by x and then y
	std::vector<std::vector<Components::Item>> grid(CHUNK_WIDTH, std::vector(WATER_LEVEL * 2, Components::AIR()));

	// Spawn blocks
	const auto offset = mPosition * CHUNK_WIDTH;
	for (std::uint64_t i = 0; i < CHUNK_WIDTH; ++i) {
		const double height = noise->getNoise(i + position * CHUNK_WIDTH);
		const std::uint64_t block_height = WATER_LEVEL + 5 * height;
		mHeightMap[i] = block_height;

		for (std::uint64_t y = 0; y < block_height; ++y) {
			grid[i][y] = Components::Item::STONE;
		}
		grid[i][block_height] = Components::Item::GRASS_BLOCK;

		// Spawn structures
		for (const auto& [chance, structure] : registers::SURFACE_STRUCTURES) {
			double roll = noise->randf();

			// Rig the roll so there is always a tree near
			if (i + offset == 3) {
				roll = 0;
			}

			if (roll < chance) {
				spawnStructure(grid, Eigen::Vector2i(i, block_height), structure, scene);
			}
		}
	}

	const auto& blockView = scene->view<Components::block>();
	for (std::uint64_t x = 0; x < CHUNK_WIDTH; ++x) {
		for (std::uint64_t y = 0; y < WATER_LEVEL * 2; ++y) {
			if (grid[x][y] == Components::AIR()) {
				continue;
			}

			const Eigen::Vector2i pos = Eigen::Vector2i(x + mPosition * CHUNK_WIDTH, y);

			Components::Item occupied = Components::AIR();
			for (const auto block : blockView) {
				if (scene->get<Components::block>(block).mPosition == pos) {
					occupied = scene->get<Components::block>(block).mType;
					break;
				}
			}

			if (occupied != Components::AIR()) {
				grid[x][y] = occupied;

				continue;
			}

			Texture* const texture =
				Game::getInstance()->getSystemManager()->getTexture(registers::TEXTURES.at(grid[x][y]));
			const EntityID entity = scene->newEntity();
			scene->emplace<Components::block>(entity, grid[x][y], Eigen::Vector2i(x + offset, y));
			scene->emplace<Components::texture>(entity, texture);

			if (registers::COLLISION_BOXES.contains(grid[x][y])) {
				const auto& collision = registers::COLLISION_BOXES.at(grid[x][y]);
				scene->emplace<Components::collision>(entity, collision.first, collision.second, true);
			} else {
				scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f),
								      texture->getSize(), true);
			}
		}
	}
}

// Loading from save
Chunk::Chunk(const rapidjson::Value& data, Scene* scene) : mPosition(data[POSITION_KEY].GetInt64()) {

	for (rapidjson::SizeType i = 0; i < data[BLOCKS_KEY].Size(); i++) {
		const Components::Item block = static_cast<Components::Item>(data[BLOCKS_KEY][i][0].GetUint64());

		SDL_assert(registers::TEXTURES.contains(block));

		Texture* const texture =
			Game::getInstance()->getSystemManager()->getTexture(registers::TEXTURES.at(block));

		const EntityID entity = scene->newEntity();
		scene->emplace<Components::block>(entity, block, getVector2i(data[BLOCKS_KEY][i][1]));
		scene->emplace<Components::texture>(entity, texture);

		if (registers::COLLISION_BOXES.contains(block)) {
			const auto& box = registers::COLLISION_BOXES.at(block);

			if (!(box.second.x() == 0 || box.second.y() == 0)) {
				scene->emplace<Components::collision>(entity, box.first, box.second, true);
			}
		} else {
			scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f), texture->getSize(),
							      true);
		}
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

void Chunk::spawnStructure(std::vector<std::vector<Components::Item>>& blocks, const Eigen::Vector2i& pos,
			   const std::vector<std::pair<Components::Item, Eigen::Vector2i>> structure,
			   Scene* const scene) {
	const auto& blockView = scene->view<Components::block>();
	for (const auto& [blockType, offset] : structure) {
		const Eigen::Vector2i realPos = pos + offset;

		if (realPos.x() < 0 || realPos.x() >= CHUNK_WIDTH) {
			// Lets place in scene
			const Eigen::Vector2i position = offset + pos + Eigen::Vector2i(mPosition * CHUNK_WIDTH, 0);

			bool occupied = false;
			for (const auto block : blockView) {
				if (scene->get<Components::block>(block).mPosition == position) {
					occupied = true;
					break;
				}
			}

			if (occupied) {
				continue;
			}

			SDL_assert(registers::BREAK_TIMES.contains(blockType) && "The block to be placed isn't brakable!");

			Texture* const texture =
				Game::getInstance()->getSystemManager()->getTexture(registers::TEXTURES.at(blockType));
			const EntityID entity = scene->newEntity();
			scene->emplace<Components::block>(entity, blockType, position);
			scene->emplace<Components::texture>(entity, texture);

			if (registers::COLLISION_BOXES.contains(blockType)) {
				const auto& collision = registers::COLLISION_BOXES.at(blockType);
				scene->emplace<Components::collision>(entity, collision.first, collision.second, true);
			} else {
				scene->emplace<Components::collision>(entity, Eigen::Vector2f(0.0f, 0.0f),
								      texture->getSize(), true);
			}
		} else {
			if (blocks[realPos.x()][realPos.y()] == Components::AIR()) {
				blocks[realPos.x()][realPos.y()] = blockType;
			}
		}
	}
}

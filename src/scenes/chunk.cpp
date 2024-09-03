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

Chunk::Chunk(Game* game, Scene* scene, const std::int64_t position) : mPosition(position) {
	mBlocks.reserve(CHUNK_WIDTH);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	// Spawn blocks
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks[i].emplace_back(scene->newEntity());
		const EntityID& entity = mBlocks[i].back();
		scene->emplace<Components::block>(entity, Components::block::STONE);
		scene->emplace<Components::texture>(entity, game->getSystemManager()->getTexture("stone.png", true));

		const auto scale = scene->get<Components::texture>(entity).mScale;

		scene->emplace<Components::position>(
			entity,
			Eigen::Vector2f((i + mPosition * 32) *
						game->getSystemManager()->getTexture("stone.png", true)->getWidth() *
						scale,
					0.0f));
		scene->emplace<Components::collision>(
			entity, Eigen::Vector2f(0.0f, 0.0f),
			Eigen::Vector2f(game->getSystemManager()->getTexture("stone.png", true)->getWidth(),
					game->getSystemManager()->getTexture("stone.png", true)->getHeight()) *
				scale,
			true);
	}

	// The second layer of grass
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks[i].emplace_back(scene->newEntity());
		const EntityID& entity = mBlocks[i].back();
		scene->emplace<Components::block>(entity, Components::block::GRASS_BLOCK);
		scene->emplace<Components::texture>(entity,
						    game->getSystemManager()->getTexture("grass-block.png", true));

		const auto scale = scene->get<Components::texture>(entity).mScale;

		scene->emplace<Components::position>(
			entity,
			Eigen::Vector2f(
				(i + mPosition * 32) *
					game->getSystemManager()->getTexture("grass-block.png", true)->getWidth() *
					scale,
				game->getSystemManager()->getTexture("grass-block.png", true)->getHeight() * scale));
		scene->emplace<Components::collision>(
			entity, Eigen::Vector2f(0.0f, 0.0f),
			Eigen::Vector2f(game->getSystemManager()->getTexture("grass-block.png", true)->getWidth(),
					game->getSystemManager()->getTexture("grass-block.png", true)->getHeight()) *
				scale,
			true);
	}
}

Chunk::Chunk(Game* game, Scene* scene, const nlohmann::json& data) : mPosition(data["position"]) {
	mBlocks.reserve(CHUNK_WIDTH);
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		mBlocks.emplace_back(std::vector<EntityID>());
	}

	// Load the things only when we need them
	std::unordered_map<Components::block::BlockType, std::function<Texture*()>> blockToTexture = {
		{Components::block::STONE, [game] { return game->getSystemManager()->getTexture("stone.png", true); }},
		{Components::block::GRASS_BLOCK,
		 [game] { return game->getSystemManager()->getTexture("grass-block.png", true); }},
	};

	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		for (std::size_t j = 0; j < data["blocks"][i].size(); ++j) {
			const auto& block = static_cast<Components::block::BlockType>(data["blocks"][i][j]);

			SDL_assert(blockToTexture.contains(block));

			Texture* texture = blockToTexture[block]();

			mBlocks[i].emplace_back(scene->newEntity());

			const EntityID& entity = mBlocks[i].back();
			scene->emplace<Components::block>(entity, block);
			scene->emplace<Components::texture>(entity, texture);

			const auto scale = scene->get<Components::texture>(entity).mScale;

			scene->emplace<Components::position>(
				entity, Eigen::Vector2f((i + mPosition * 32) * texture->getWidth() * scale,
							(j)*texture->getWidth() * scale));
			scene->emplace<Components::collision>(
				entity, Eigen::Vector2f(0.0f, 0.0f),
				Eigen::Vector2f(texture->getWidth(), texture->getHeight()) * scale, true);
		}
	}
}

Chunk::~Chunk() { SDL_assert(mBlocks.empty() && "Didn't save?"); }

nlohmann::json Chunk::save(Scene* scene) {
	nlohmann::json chunk;

	chunk["position"] = mPosition;
	for (auto i = 0; i < CHUNK_WIDTH; ++i) {
		for (std::size_t j = 0; j < mBlocks[i].size(); ++j) {
			chunk["blocks"][i][j] =
				static_cast<std::uint64_t>(scene->get<Components::block>(mBlocks[i][j]).mType);
		}
	}

	// Only clear when assert is enabled
	SDL_assert((mBlocks.clear(), true));

	return chunk;
}

#pragma once

#include "managers/entityManager.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <vector>

// TODO: Random noise
class Chunk {
      public:
	inline constexpr const static int MAX_HEIGHT = 128;
	inline constexpr const static int CHUNK_WIDTH = 16;
	inline constexpr const static int WATER_LEVEL = 16;

	// Generate a chunk from scratch
	Chunk(class Game* game, class Scene* scene, const std::int64_t position);
	// Load from json
	Chunk(class Game* game, class Scene* scene, const nlohmann::json& data);
	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	~Chunk() = default;

	[[nodiscard]] nlohmann::json save(class Scene* scene);

	[[nodiscard]] std::int64_t getPosition() const { return mPosition; }

      private:
	const std::int64_t mPosition;
	std::vector<std::vector<EntityID>> mBlocks;
};

#pragma once

#include "components.hpp"
#include "third_party/json.hpp"

#include <cstdint>

// TODO: Random noise
class Chunk {
      public:
	inline constexpr const static int MAX_HEIGHT = 128;
	inline constexpr const static int CHUNK_WIDTH = 16;
	inline constexpr const static int WATER_LEVEL = 16;

	// Generate a chunk from scratch
	explicit Chunk(class Game* game, class Scene* scene, const class NoiseGenerator* const noise,
		       const std::int64_t position);
	// Load from json
	explicit Chunk(const nlohmann::json& data, class Game* game, class Scene* scene);
	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	~Chunk() = default;

	[[nodiscard]] nlohmann::json save(class Scene* scene);

	[[nodiscard]] std::int64_t getPosition() const { return mPosition; }

      private:
	constexpr const static inline char* const POSITION_KEY = "position";
	constexpr const static inline char* const BLOCKS_KEY = "blocks";
	constexpr const static inline char* const ITEMS_KEY = "items";

	void spawnStructure(const Eigen::Vector2i& pos,
			    const std::vector<std::pair<Components::Item, Eigen::Vector2i>> structure,
			    class Scene* scene);

	const std::int64_t mPosition;
	std::array<std::uint64_t, CHUNK_WIDTH> mHeightMap;
	Game* mGame;
};

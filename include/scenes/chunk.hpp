#pragma once

#include "components.hpp"
#include "third_party/rapidjson/document.h"

#include <cstdint>

class Chunk {
      public:
	inline constexpr const static int MAX_HEIGHT = 128;
	inline constexpr const static int CHUNK_WIDTH = 16;
	inline constexpr const static int WATER_LEVEL = 16;

	// Generate a chunk from scratch
	explicit Chunk(class Scene* scene, class NoiseGenerator* const noise, const std::int64_t position);
	// Load from json
	explicit Chunk(const rapidjson::Value& data, class Scene* scene);

	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	~Chunk() = default;

	void save(class Scene* scene, rapidjson::Value& chunk, rapidjson::MemoryPoolAllocator<>& allocator);

	[[nodiscard]] std::int64_t getPosition() const { return mPosition; }

      private:
	constexpr const static inline char* const POSITION_KEY = "position";
	constexpr const static inline char* const BLOCKS_KEY = "blocks";
	constexpr const static inline char* const ITEMS_KEY = "items";

	void spawnStructure(std::vector<std::vector<Components::Item>>& blocks, const Eigen::Vector2i& pos,
			    const std::vector<std::pair<Components::Item, Eigen::Vector2i>> structure,
			    class Scene* scene);
	void carve(std::vector<std::vector<Components::Item>>& blocks, class NoiseGenerator* const noise);
	void spawnOres(std::vector<std::vector<Components::Item>>& blocks, class NoiseGenerator* const noise);

	const std::int64_t mPosition;
	std::array<std::uint64_t, CHUNK_WIDTH> mHeightMap;
};

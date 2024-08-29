#pragma once

#include "misc/blocks.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <vector>

class Chunk {
      public:
	// Generate
	Chunk(class Scene* scene, const std::int64_t position);
	// Load
	Chunk(class Scene* scene, nlohmann::json data);
	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	// Delete all the block that we generated
	~Chunk();

      private:
	constexpr const static int maxHeight = 128;

	const std::int64_t mPosition;
	std::vector<std::vector<Block>> mBlocks;
};

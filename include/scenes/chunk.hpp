#pragma once

#include "managers/entityManager.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <vector>

class Chunk {
      public:
	// Generate
	Chunk(class Game* game, class Scene* scene, const std::int64_t position);
	// Load
	Chunk(class Game* game, class Scene* scene, const nlohmann::json& data);
	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	// Delete all the blocks that we generated
	~Chunk();

	nlohmann::json save(class Scene* scene);

      private:
	constexpr const static int maxHeight = 128;

	const std::int64_t mPosition;
	std::vector<std::vector<EntityID>> mBlocks;
};

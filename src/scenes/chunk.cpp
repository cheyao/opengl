#include "scenes/chunk.hpp"

#include "scene.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <vector>

Chunk::Chunk(Scene* scene, const std::int64_t position) : mPosition(position) {
	mBlocks.reserve(32);
	for (size_t i = 0; i < 32; ++i) {
		mBlocks.emplace_back(std::vector<Block>());
	}
}

Chunk::Chunk(Scene* scene, nlohmann::json data) : mPosition(data["position"]) {
	mBlocks.reserve(32);
	for (size_t i = 0; i < 32; ++i) {
		mBlocks.emplace_back(std::vector<Block>());
	}
}

Chunk::~Chunk() {}


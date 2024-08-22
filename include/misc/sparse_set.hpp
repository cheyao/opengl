#pragma once

#include "managers/entityManager.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

// PERF: https://gist.github.com/dakom/82551fff5d2b843cbe1601bbaff2acbf
// FIXME: This is probably not the best implementation
template <typename Component> class sparse_set {
      public:
	constexpr const static size_t max_size = std::numeric_limits<std::uint64_t>::max();

	sparse_set();
	sparse_set(sparse_set&&) = delete;
	sparse_set(const sparse_set&) = delete;
	sparse_set& operator=(sparse_set&&) = delete;
	sparse_set& operator=(const sparse_set&) = delete;
	~sparse_set() = default; // TODO:

	template <typename... Args> void emplace(const EntityID entity, Args&&... args);

      private:
	// Index is entity ID, value is ptr to packed container
	std::vector<uintptr_t> mSparseContainer;
	// Value is entity ID, index is ptr to component
	std::vector<std::uint64_t> mPackedContainer;
	// The real values
	std::vector<Component> mComponents;
	// The component storage
	size_t mHead;
};

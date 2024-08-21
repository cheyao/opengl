#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

class sparse_set {
      public:
	std::vector<uintptr_t> mSparseContainer;
	std::vector<std::uint64_t> mPackedContainer;
	constexpr const static size_t max_size = std::numeric_limits<std::uint64_t>::max();

	sparse_set();
	sparse_set(sparse_set&&) = delete;
	sparse_set(const sparse_set&) = delete;
	sparse_set& operator=(sparse_set&&) = delete;
	sparse_set& operator=(const sparse_set&) = delete;
	~sparse_set();

      private:
};

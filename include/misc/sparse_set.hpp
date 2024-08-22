#pragma once

#include "managers/entityManager.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

template <typename Container> struct sparse_set_iterator final {
	sparse_set_iterator() {}

	[[nodiscard]] EntityID& operator*() const {
		// give back the entityID we're currently at
	}

	[[nodiscard]] bool operator==(const Iterator& other) const {
		// Compare two iterators
	}

	[[nodiscar]] ool operator!=(const Iterator& other) const {
		// Similar to above
	}

	[[nodiscar]] terator& operator++() {
		// Move the iterator forward
	}

      private:
	const Container* packed;
	size_t offset;
};

// PERF: https://gist.github.com/dakom/82551fff5d2b843cbe1601bbaff2acbf
// FIXME: This is probably not the best implementation
template <typename Component> class sparse_set {
	using iterator = sparse_set_iterator<std::vector<EntityID>>;

      public:
	constexpr const static size_t max_size = std::numeric_limits<std::uint64_t>::max();

	sparse_set() = default;
	sparse_set(sparse_set&&) = delete;
	sparse_set(const sparse_set&) = delete;
	sparse_set& operator=(sparse_set&&) = delete;
	sparse_set& operator=(const sparse_set&) = delete;
	~sparse_set() = default; // TODO:

	template <typename... Args> void emplace(const EntityID entity, Args&&... args) {
		while (mSparseContainer.size() <= entity) {
			mSparseContainer.emplace_back(0);
		}

		mSparseContainer[entity] = mPackedContainer.size();
		mPackedContainer.emplace_back(entity);
		mComponents.emplace_back(args...);
	}

	Component& get(const EntityID entity) { return mComponents[mSparseContainer[entity]]; }

      private:
	// Index is entity ID, value is ptr to packed container
	std::vector<uintptr_t> mSparseContainer;
	// Value is entity ID, index is ptr to component
	std::vector<EntityID> mPackedContainer;
	// The real values
	std::vector<Component> mComponents;
	// The component storage
	size_t mHead;
};

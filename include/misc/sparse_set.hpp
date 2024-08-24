#pragma once

#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

// NOTE: Travers the thing from the end
template <typename Container> struct sparse_set_iterator final {
	constexpr sparse_set_iterator() noexcept : mPacked(), mOffset() {}
	constexpr sparse_set_iterator(const Container& ref, const size_t idx) noexcept : mPacked(&ref), mOffset(idx) {}

	[[nodiscard]] constexpr sparse_set_iterator& operator++() noexcept {
		--mOffset;
		return *this;
	}

	[[nodiscard]] constexpr sparse_set_iterator& operator--() noexcept {
		++mOffset;
		return *this;
	}

	[[nodiscard]] constexpr const EntityID& operator[](const size_t value) const noexcept {
		return (*mPacked)[index() - value];
	}

	[[nodiscard]] constexpr const EntityID& operator*() const noexcept { return operator[](0); }

	[[nodiscard]] constexpr size_t index() const noexcept { return mOffset - 1; }

      private:
	const Container* mPacked;
	size_t mOffset;
};

template <typename Container>
[[nodiscard]] bool operator==(const sparse_set_iterator<Container>& lhs,
			      const sparse_set_iterator<Container>& rhs) noexcept {
	return lhs.index() == rhs.index();
}

template <typename Container>
[[nodiscard]] bool operator!=(const sparse_set_iterator<Container>& lhs,
			      const sparse_set_iterator<Container>& rhs) noexcept {
	return !(lhs == rhs);
}

// PERF: https://gist.github.com/dakom/82551fff5d2b843cbe1601bbaff2acbf
// FIXME: This is probably not the best implementation
template <typename Component> class sparse_set {
	using iterator = sparse_set_iterator<std::vector<EntityID>>;

      public:
	constexpr const static size_t max_size = std::numeric_limits<std::uint64_t>::max();

	// No need to clean up, everything is in a vector
	sparse_set() = default;
	sparse_set(sparse_set&&) = delete;
	sparse_set(const sparse_set&) = delete;
	sparse_set& operator=(sparse_set&&) = delete;
	sparse_set& operator=(const sparse_set&) = delete;
	~sparse_set() = default;

	template <typename... Args> void emplace(const EntityID entity, Args&&... args) {
		while (mSparseContainer.size() <= entity) {
			mSparseContainer.emplace_back(0);
		}

		if (mSparseContainer[entity] != 0) {
			SDL_Log("\033[93mSparse_set.cpp: Container already contains entity %llu!\033[0m", entity);
		}

		mSparseContainer[entity] = mPackedContainer.size();
		mPackedContainer.emplace_back(entity);
		mComponents.emplace_back(args...);
	}

	[[nodiscard]] Component& get(const EntityID entity) noexcept {
		return mComponents[mSparseContainer[entity]];
	}

	[[nodiscard]] bool contains(const EntityID entity) const {
		if (entity >= mSparseContainer.size() || mSparseContainer[entity] >= mPackedContainer.size()) {
			return false;
		}

		return mPackedContainer[mSparseContainer[entity]] == entity;
	}

	[[nodiscard]] size_t size() const noexcept { return mPackedContainer.size(); }

	[[nodiscard]] iterator begin() const noexcept { return iterator{mPackedContainer, mPackedContainer.size()}; }

	[[nodiscard]] iterator end() const noexcept { return iterator{mPackedContainer, {}}; }

	[[nodiscard]] EntityID* data() noexcept { return mPackedContainer.data(); }

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

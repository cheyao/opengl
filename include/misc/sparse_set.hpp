#pragma once

#include "managers/entityManager.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

// NOTE: Travers the thing from the end
template <typename Container> struct sparse_set_iterator final {
	constexpr sparse_set_iterator() noexcept : packed(), offset() {}
	constexpr sparse_set_iterator(const Container& ref, const size_t idx) noexcept : packed(&ref), offset(idx) {}

	[[nodiscard]] constexpr sparse_set_iterator& operator++() noexcept {
		--offset;
		return *this;
	}

	[[nodiscard]] constexpr sparse_set_iterator& operator--() noexcept {
		++offset;
		return *this;
	}

	[[nodiscard]] constexpr EntityID& operator[](const size_t value) const noexcept {
		return (*packed)[index() - value];
	}

	[[nodiscard]] constexpr EntityID& operator*() const noexcept { return operator[](0); }

	[[nodiscard]] constexpr size_t index() const noexcept { return offset - 1; }

      private:
	const Container* packed;
	size_t offset;
};

template <typename Container>
[[nodiscard]] bool operator==(const sparse_set_iterator<Container>& lhs,
			      const sparse_set_iterator<Container>& rhs) noexcept {
	// Compare two iterators
	return lhs.index() == rhs.index();
}

template <typename Container>
[[nodiscard]] bool operator!=(const sparse_set_iterator<Container>& lhs,
			      const sparse_set_iterator<Container>& rhs) noexcept {
	// Similar to above
	return !(lhs == rhs);
}

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

	[[nodiscard]] iterator begin() const noexcept { return iterator{mPackedContainer, mPackedContainer.size()}; }

	[[nodiscard]] iterator end() const noexcept { return iterator{mPackedContainer, {}}; }

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

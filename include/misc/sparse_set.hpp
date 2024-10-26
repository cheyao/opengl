#pragma once

#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <cinttypes>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace utils {

// TODO: Tests
// NOTE: Travers the thing from the end
template <typename T> struct sparse_set_iterator final {
      public:
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;
	using value_type = T;
	using reference = value_type&;
	using const_reference = const value_type&;

	constexpr sparse_set_iterator() noexcept : mPacked(nullptr), mOffset(0) {}
	constexpr sparse_set_iterator(T& ref, const std::size_t idx) noexcept : mPacked(&ref), mOffset(idx) {}

	constexpr sparse_set_iterator(const sparse_set_iterator& other)
		: mPacked(other.mPacked), mOffset(other.mOffset) {}
	constexpr sparse_set_iterator& operator=(const sparse_set_iterator& other) {
		mPacked = other.mPacked;
		mOffset = other.mOffset;
		return *this;
	}

	constexpr ~sparse_set_iterator() = default;

	constexpr sparse_set_iterator& operator++() noexcept {
		--mOffset;
		return *this;
	}
	constexpr sparse_set_iterator operator++(int) noexcept {
		sparse_set_iterator old(*this);
		operator++();
		return old;
	}

	constexpr sparse_set_iterator& operator--() noexcept {
		++mOffset;
		return *this;
	}
	constexpr sparse_set_iterator operator--(int) noexcept {
		sparse_set_iterator old(*this);
		operator--();
		return old;
	}

	[[nodiscard]] constexpr const T& operator[](const std::size_t value) const noexcept {
		return (*mPacked)[index() - value];
	}

	[[nodiscard]] constexpr const T& operator*() const noexcept { return operator[](0); }

	[[nodiscard]] constexpr std::size_t index() const noexcept { return mOffset - 1; }

      private:
	T* mPacked;
	std::size_t mOffset;
};

template <typename T>
[[nodiscard]] bool operator==(const sparse_set_iterator<T>& lhs, const sparse_set_iterator<T>& rhs) noexcept {
	return lhs.index() == rhs.index();
}

template <typename T>
[[nodiscard]] bool operator!=(const sparse_set_iterator<T>& lhs, const sparse_set_iterator<T>& rhs) noexcept {
	return !(lhs == rhs);
}

template <typename T>
[[nodiscard]] bool operator<(const sparse_set_iterator<T>& lhs, const sparse_set_iterator<T>& rhs) noexcept {
	return lhs.index() > rhs.index();
}
template <typename T>
[[nodiscard]] bool operator>(const sparse_set_iterator<T>& lhs, const sparse_set_iterator<T>& rhs) {
	return rhs < lhs;
}
template <typename T>
[[nodiscard]] bool operator<=(const sparse_set_iterator<T>& lhs, const sparse_set_iterator<T>& rhs) {
	return !(lhs > rhs);
}
template <typename T>
[[nodiscard]] bool operator>=(const sparse_set_iterator<T>& lhs, const sparse_set_iterator<T>& rhs) {
	return !(lhs < rhs);
}

class sparse_set_interface {
      public:
	using iterator = sparse_set_iterator<EntityID>;
	using const_iterator = sparse_set_iterator<const EntityID>;

	virtual ~sparse_set_interface() = 0;
	[[nodiscard]] virtual bool contains(EntityID entity) const noexcept = 0;
	virtual void erase(const EntityID entity) noexcept = 0;
	[[nodiscard]] virtual std::size_t size() const noexcept = 0;
	[[nodiscard]] virtual EntityID* data() noexcept = 0;

	[[nodiscard]] virtual iterator begin() noexcept = 0;
	[[nodiscard]] virtual const_iterator begin() const noexcept = 0;
	[[nodiscard]] virtual const_iterator cbegin() const noexcept = 0;

	[[nodiscard]] virtual iterator end() noexcept = 0;
	[[nodiscard]] virtual const_iterator end() const noexcept = 0;
	[[nodiscard]] virtual const_iterator cend() const noexcept = 0;
};

// Prevent the destructor to crash the program due to polymorphism
inline sparse_set_interface::~sparse_set_interface() {}

// PERF: https://gist.github.com/dakom/82551fff5d2b843cbe1601bbaff2acbf
// FIXME: This is probably not the best implementation
template <typename Component> class sparse_set : public sparse_set_interface {
      public:
	using iterator = sparse_set_interface::iterator;
	using const_iterator = sparse_set_interface::const_iterator;

	constexpr const static std::size_t max_size = std::numeric_limits<std::uint64_t>::max();

	// No need to clean up, everything is in a vector
	sparse_set() = default;
	sparse_set(sparse_set&&) = delete;
	sparse_set(const sparse_set&) = delete;
	sparse_set& operator=(sparse_set&&) = delete;
	sparse_set& operator=(const sparse_set&) = delete;
	~sparse_set() override {}

	template <typename... Args> void emplace(const EntityID entity, Args&&... args) {
		while (mSparseContainer.size() <= entity) {
			mSparseContainer.emplace_back(0);
		}

		if (mSparseContainer[entity] != 0) {
			SDL_Log("\033[93mSparse_set.cpp: Container already contains entity %" PRIu64 "!\033[0m",
				entity);
		}

		mSparseContainer[entity] = mPackedContainer.size();
		mPackedContainer.emplace_back(entity);
		mComponents.emplace_back(args...);
	}

	[[nodiscard]] Component& get(const EntityID entity) noexcept {
#ifdef DEBUG
		if (!contains(entity)) {
			SDL_LogCritical(
				SDL_LOG_CATEGORY_VIDEO,
				"\x1B[31msparse_set.hpp: Error! Accessing invalid component %s for entity %" PRIu64
				"\033[0m",
				typeid(Component).name(), entity);

			SDL_assert(contains(entity) && "Hey! This sparse set doesn't contain this entity");
		}
#endif

		return mComponents[mSparseContainer[entity]];
	}

	[[nodiscard]] bool contains(const EntityID entity) const noexcept override {
		if (entity >= mSparseContainer.size() || mSparseContainer[entity] >= mPackedContainer.size()) {
			return false;
		}

		return mPackedContainer[mSparseContainer[entity]] == entity;
	}

	[[nodiscard]] std::size_t size() const noexcept override { return mPackedContainer.size(); }

	[[nodiscard]] iterator begin() noexcept override {
		return iterator{*mPackedContainer.data(), mPackedContainer.size()};
	}
	[[nodiscard]] const_iterator begin() const noexcept override {
		return const_iterator{*mPackedContainer.data(), mPackedContainer.size()};
	}
	[[nodiscard]] const_iterator cbegin() const noexcept override {
		return const_iterator{*mPackedContainer.data(), mPackedContainer.size()};
	}

	[[nodiscard]] iterator end() noexcept override { return iterator{*mPackedContainer.data(), 0}; }
	[[nodiscard]] const_iterator end() const noexcept override {
		return const_iterator{*mPackedContainer.data(), 0};
	}
	[[nodiscard]] const_iterator cend() const noexcept override {
		return const_iterator{*mPackedContainer.data(), 0};
	}

	[[nodiscard]] bool empty() const noexcept { return mPackedContainer.empty(); }

	[[nodiscard]] EntityID* data() noexcept override { return mPackedContainer.data(); }

	void erase(const EntityID entity) noexcept override {
		// https://gist.github.com/dakom/82551fff5d2b843cbe1601bbaff2acbf
		mSparseContainer[mPackedContainer.back()] = mSparseContainer[entity];
		mPackedContainer[mSparseContainer[entity]] = mPackedContainer.back();
		mComponents[mSparseContainer[entity]] = mComponents.back();
		mPackedContainer.pop_back();
		mComponents.pop_back();

		mSparseContainer[entity] = 0; // 1. The index of EntityIndices, equal to the value of the
					      // entity, is removed (leaving a hole)
	}

      private:
	// Index is entity ID, value is ptr to packed container
	std::vector<std::uintptr_t> mSparseContainer;
	// Value is entity ID, index is ptr to component
	std::vector<EntityID> mPackedContainer;
	// The real values
	std::vector<Component> mComponents;
	// The component storage
	std::size_t mHead;
};

} // namespace utils

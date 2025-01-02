#pragma once

#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <cinttypes>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace utils {

// TODO: Put common stuff here
class sparse_set_interface {
	using underlying_container = std::vector<EntityID>;
      public:
	using iterator = underlying_container::iterator;
	using const_iterator = underlying_container::const_iterator;

	virtual ~sparse_set_interface() = 0;
	[[nodiscard]] virtual bool contains(EntityID entity) const noexcept = 0;
	virtual void erase(const EntityID entity) noexcept = 0;
	virtual void clear() = 0;
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
	using underlying_container = std::vector<EntityID>;
      public:
	using iterator = underlying_container::iterator;
	using const_iterator = underlying_container::const_iterator;

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
		mComponents.emplace_back(std::forward<Args>(args)...);
	}

	[[nodiscard]] Component& get(const EntityID entity) noexcept {
#ifdef DEBUG
		if (!contains(entity)) {
#ifndef __EMSCRIPTEN__ // No typeid
			SDL_LogCritical(
				SDL_LOG_CATEGORY_VIDEO,
				"\x1B[31msparse_set.hpp: Error! Accessing invalid component %s for entity %" PRIu64
				"\033[0m",
				typeid(Component).name(), entity);
#endif

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
		return mPackedContainer.begin();
		// return iterator{*mPackedContainer.data(), 0};
	}
	[[nodiscard]] const_iterator begin() const noexcept override {
		return mPackedContainer.begin();
		// return const_iterator{*mPackedContainer.data(), 0};
	}
	[[nodiscard]] const_iterator cbegin() const noexcept override {
		return mPackedContainer.cbegin();
		// return const_iterator{*mPackedContainer.data(), 0};
	}

	[[nodiscard]] iterator end() noexcept override { 
		return mPackedContainer.end();
		// return iterator{*mPackedContainer.data(), mPackedContainer.size()}; 
	}
	[[nodiscard]] const_iterator end() const noexcept override {
		return mPackedContainer.end();
		// return const_iterator{*mPackedContainer.data(), mPackedContainer.size()};
	}
	[[nodiscard]] const_iterator cend() const noexcept override {
		return mPackedContainer.cend();
		// return const_iterator{*mPackedContainer.data(), mPackedContainer.size()};
	}

	[[nodiscard]] bool empty() const noexcept { return mPackedContainer.empty(); }

	[[nodiscard]] EntityID* data() noexcept override { return mPackedContainer.data(); }

	constexpr void clear() override {
		mPackedContainer.clear();
		mSparseContainer.clear();
		mComponents.clear();
	}

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
	underlying_container mPackedContainer;
	// The real values
	std::vector<Component> mComponents;
};

} // namespace utils

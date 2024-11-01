#pragma once

#include "imgui.h"
#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace utils {

template <typename Iterator> struct iterable_adaptor final {
	constexpr iterable_adaptor(Iterator from, Iterator to) noexcept : first{std::move(from)}, last{std::move(to)} {}

	[[nodiscard]] constexpr Iterator begin() const noexcept { return first; }
	[[nodiscard]] constexpr Iterator cbegin() const noexcept { return begin(); }

	[[nodiscard]] constexpr Iterator end() const noexcept { return last; }
	[[nodiscard]] constexpr Iterator cend() const noexcept { return end(); }

      private:
	Iterator first;
	Iterator last;
};

template <typename... ComponentTypes> class sparse_set_view_tuple_iterator final {
      public:
	// mEntities is copied, but it shouldn't - bad for perf
	sparse_set_view_tuple_iterator(ComponentManager* componentManager, const std::vector<EntityID>& entities,
				       std::size_t offset) noexcept
		: mComponentManager(componentManager), mEntities(entities), mOffset(offset) {}

	sparse_set_view_tuple_iterator& operator++() noexcept { return ++mOffset, *this; }

	sparse_set_view_tuple_iterator& operator--() noexcept { return --mOffset, *this; }

	[[nodiscard]] decltype(auto) operator[](const size_t value) const noexcept {
		return std::make_tuple(mEntities[index() + value], mComponentManager->getPool<ComponentTypes>()->get(
									   mEntities[index() + value])...);
	}

	[[nodiscard]] decltype(auto) operator*() const noexcept { return operator[](0); }

	[[nodiscard]] size_t index() const noexcept { return mOffset; }

      protected:
	class ComponentManager* mComponentManager;
	const std::vector<EntityID> mEntities;
	std::size_t mOffset;
};

template <typename... ComponentTypes>
[[nodiscard]] bool operator==(const sparse_set_view_tuple_iterator<ComponentTypes...>& lhs,
			      const sparse_set_view_tuple_iterator<ComponentTypes...>& rhs) noexcept {
	return lhs.index() == rhs.index();
}

template <typename... ComponentTypes>
[[nodiscard]] bool operator!=(const sparse_set_view_tuple_iterator<ComponentTypes...>& lhs,
			      const sparse_set_view_tuple_iterator<ComponentTypes...>& rhs) noexcept {
	return !(lhs == rhs);
}

template <typename... ComponentTypes> class sparse_set_view {
      public:
	using iterator = std::vector<EntityID>::iterator;
	using const_iterator = std::vector<EntityID>::const_iterator;
	using tuple_iterator = sparse_set_view_tuple_iterator<ComponentTypes...>;
	using iterable = iterable_adaptor<sparse_set_view_tuple_iterator<ComponentTypes...>>;

	// Please don't touch this, took a long time to figure out
	sparse_set_view(ComponentManager* componentManager) : mComponentManager(componentManager) {
		SDL_COMPILE_TIME_ASSERT(sizeof...(ComponentTypes) != 0, "No empty views!");

		// This first part makes a array of all the sizes of the that we loop through
		const std::array<utils::sparse_set_interface*, sizeof...(ComponentTypes)> sets = {
			mComponentManager->getPool<ComponentTypes>()...};
		const auto& smallest = **std::ranges::min_element(
			sets, [](const auto& a, const auto& b) { return a->size() < b->size(); });

		for (const auto& id : std::span<const EntityID>(smallest)) {
			// This creates a vector of bools for every pool which represent if the pool contains the entity
			if ((... && (mComponentManager->getPool<ComponentTypes>()->contains(id)))) {
				mEntities.emplace_back(id);
			}
		}
		// Now at the end of the program, all the entites that are present in all the components are present in
		// mEntities (doesn't count in addition and deletion)
	}

	sparse_set_view(const sparse_set_view& other) noexcept
		: mComponentManager(other.mComponentManager), mEntities(other.mEntities) {}
	sparse_set_view& operator=(const sparse_set_view& other) noexcept {
		mComponentManager = other.mComponentManager;
		mEntities = other.mEntities;
	}

	sparse_set_view(sparse_set_view&& other) noexcept
		: mComponentManager(other.mComponentManager), mEntities(std::move(other.mEntities)) {}
	sparse_set_view& operator=(sparse_set_view&& other) noexcept {
		mComponentManager = other.mComponentManager;
		mEntities = std::move(other.mEntities);
	}

	~sparse_set_view() {}

	[[nodiscard]] iterator begin() noexcept { return mEntities.begin(); }
	[[nodiscard]] const_iterator begin() const noexcept { return mEntities.begin(); }
	[[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

	[[nodiscard]] iterator end() noexcept { return mEntities.end(); }
	[[nodiscard]] const_iterator end() const noexcept { return mEntities.end(); }
	[[nodiscard]] const_iterator cend() const noexcept { return end(); }

	[[nodiscard]] iterable each() const noexcept {
		return iterable{tuple_iterator{mComponentManager, mEntities, 0},
				tuple_iterator{mComponentManager, mEntities, mEntities.size()}};
	}

	template <typename... Components> [[nodiscard]] decltype(auto) get(const EntityID entt) const {
		if constexpr (sizeof...(Components) == 1) {
			return (mComponentManager->getPool<Components>()->get(entt), ...);
		} else {
			return std::make_tuple(mComponentManager->getPool<Components>()->get(entt)...);
		}
	}

	template <typename Func> void each(Func func) const {
		if constexpr (std::is_invocable_v<Func, EntityID>) {
			for (const auto entity : mEntities) {
				func(entity);
			}
		} else if constexpr (std::is_invocable_v<Func, EntityID, ComponentTypes&...>) {
			for (const auto entity : mEntities) {
				func(entity, mComponentManager->getPool<ComponentTypes>()->get(entity)...);
			}
		} else if constexpr (std::is_invocable_v<Func, ComponentTypes&...>) {
			for (const auto entity : mEntities) {
				func(mComponentManager->getPool<ComponentTypes>()->get(entity)...);
			}
		} else {
			static_assert(false, "The signatures for each are: (EntityID), (EntityID, ComponentTypes&...) "
					     "and (ComponentTypes&...)");
			std::unreachable();
		}
	}

	std::size_t size() const noexcept { return mEntities.size(); }

	constexpr EntityID* data() noexcept { return mEntities.data(); }
	constexpr const EntityID* data() const noexcept { return mEntities.data(); }

      private:
	class ComponentManager* const mComponentManager;
	std::vector<EntityID> mEntities;
};

} // namespace utils

#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

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

template <typename... ComponentTypes> class sparse_set_view_iterator {
      public:
	sparse_set_view_iterator(ComponentManager* componentManager, const std::vector<EntityID>& entities,
				 size_t offset) noexcept
		: mComponentManager(componentManager), mEntities(entities), mOffset(offset) {}

	sparse_set_view_iterator& operator++() noexcept {
		++mOffset;
		return *this;
	}

	sparse_set_view_iterator& operator--() noexcept {
		--mOffset;
		return *this;
	}

	[[nodiscard]] const EntityID& operator[](const size_t value) const noexcept {
		return mEntities[index() + value];
	}

	[[nodiscard]] const EntityID& operator*() const noexcept { return operator[](0); }

	[[nodiscard]] size_t index() const noexcept { return mOffset; }

      protected:
	class ComponentManager* mComponentManager;
	const std::vector<EntityID> mEntities;
	size_t mOffset;
};

template <typename... ComponentTypes>
[[nodiscard]] bool operator==(const sparse_set_view_iterator<ComponentTypes...>& lhs,
			      const sparse_set_view_iterator<ComponentTypes...>& rhs) noexcept {
	return lhs.index() == rhs.index();
}

template <typename... ComponentTypes>
[[nodiscard]] bool operator!=(const sparse_set_view_iterator<ComponentTypes...>& lhs,
			      const sparse_set_view_iterator<ComponentTypes...>& rhs) noexcept {
	return !(lhs == rhs);
}

template <typename... ComponentTypes> class sparse_set_view_tuple_iterator {
      public:
	// mEntities is copied, but it shouldn't - bad for perf
	sparse_set_view_tuple_iterator(ComponentManager* componentManager, const std::vector<EntityID>& entities,
				       size_t offset) noexcept
		: mComponentManager(componentManager), mEntities(entities), mOffset(offset) {}

	sparse_set_view_tuple_iterator& operator++() noexcept {
		++mOffset;
		return *this;
	}

	sparse_set_view_tuple_iterator& operator--() noexcept {
		--mOffset;
		return *this;
	}

	[[nodiscard]] decltype(auto) operator[](const size_t value) const noexcept {
		return std::make_tuple(mEntities[index() + value], mComponentManager->getPool<ComponentTypes>()->get(
									   mEntities[index() + value])...);
	}

	[[nodiscard]] decltype(auto) operator*() const noexcept { return operator[](0); }

	[[nodiscard]] size_t index() const noexcept { return mOffset; }

      protected:
	class ComponentManager* mComponentManager;
	const std::vector<EntityID> mEntities;
	size_t mOffset;
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
	using iterator = sparse_set_view_iterator<ComponentTypes...>;
	using tuple_iterator = sparse_set_view_tuple_iterator<ComponentTypes...>;
	using iterable = iterable_adaptor<sparse_set_view_tuple_iterator<ComponentTypes...>>;

      public:
	// Please don't touch this, took a long time to figure out
	sparse_set_view(ComponentManager* componentManager) : mComponentManager(componentManager) {
		// This first part makes a vector of all the sizes of the components
		const std::vector<size_t> sizes = {mComponentManager->getPool<ComponentTypes>()->size()...};

		// This second part loops over all the sizes of in the vector and picks out the smallest
		size_t smallestSize = std::numeric_limits<std::size_t>::max();
		size_t smallestIndex = std::numeric_limits<std::size_t>::max();

		for (size_t i = 0; i < sizes.size(); ++i) {
			if (sizes[i] <= smallestSize) {
				smallestSize = sizes[i];
				smallestIndex = i;
			}
		}

		assert(smallestSize != std::numeric_limits<std::size_t>::max());
		assert(smallestIndex != std::numeric_limits<std::size_t>::max());

		// This should be the same
		assert(smallestSize == sizes[smallestIndex]);

		// Now we are looping through all the entity ids of the the smallest component pool
		// FIXME: This is __NOT__ good code
		const std::vector<EntityID*> ids = {mComponentManager->getPool<ComponentTypes>()->data()...};
		for (const auto& id : std::span<EntityID>(ids[smallestIndex], smallestSize)) {
			// This creates a vector of bools for every pool which represent if the pool contains the entity
			const std::vector<bool> poolContains = {
				mComponentManager->getPool<ComponentTypes>()->contains(id)...};

			// If it does add the entity to our view's pool
			if (std::all_of(poolContains.begin(), poolContains.end(), [](const bool v) { return v; })) {
				mEntities.emplace_back(id);
			}
		}
		// Now at the end of the program, all the entites that are present in all the components are present in
		// mEntities (doesn't count in addition and deletion)
	}

	sparse_set_view(sparse_set_view&&) = delete;
	sparse_set_view(const sparse_set_view&) = delete;
	sparse_set_view& operator=(sparse_set_view&&) = delete;
	sparse_set_view& operator=(const sparse_set_view&) = delete;
	~sparse_set_view() {}

	[[nodiscard]] iterator begin() const noexcept { return iterator{mComponentManager, mEntities, 0}; }
	[[nodiscard]] const iterator cbegin() const noexcept { return begin(); }

	[[nodiscard]] iterator end() const noexcept { return iterator{mComponentManager, mEntities, mEntities.size()}; }
	[[nodiscard]] const iterator cend() const noexcept { return end(); }

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
		} else {
			for (const auto entity : mEntities) {
				func(mComponentManager->getPool<ComponentTypes>()->get(entity)...);
			}
		}
	}

	std::size_t size() const noexcept { return mEntities.size(); }

      private:
	class ComponentManager* mComponentManager;
	std::vector<EntityID> mEntities;
};

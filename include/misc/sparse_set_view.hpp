#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"

#include <algorithm>
#include <span>
#include <vector>

template <typename... ComponentTypes> class sparse_set_view_iterator {
      public:
	sparse_set_view_iterator(ComponentManager* componentManager, const std::vector<EntityID>& entities,
				 size_t offset) noexcept
		: mComponentManager(componentManager), mEntities(&entities), mOffset(offset) {}

	sparse_set_view_iterator& operator++() noexcept {
		++mOffset;
		return *this;
	}

	sparse_set_view_iterator& operator--() noexcept {
		--mOffset;
		return *this;
	}

	[[nodiscard]] const EntityID& operator[](const size_t value) const noexcept {
		return (*mEntities)[index() + value];
	}

	[[nodiscard]] const EntityID& operator*() const noexcept { return operator[](0); }

	[[nodiscard]] size_t index() const noexcept { return mOffset; }

      private:
	class ComponentManager* mComponentManager;
	const std::vector<EntityID>* mEntities;
	size_t mOffset;
};

template <typename... ComponentTypes>
[[nodiscard]] bool operator==(const sparse_set_view_iterator<ComponentTypes...>& lhs,
			      const sparse_set_view_iterator<ComponentTypes...>& rhs) noexcept {
	// Compare two iterators
	return lhs.index() == rhs.index();
}

template <typename... ComponentTypes>
[[nodiscard]] bool operator!=(const sparse_set_view_iterator<ComponentTypes...>& lhs,
			      const sparse_set_view_iterator<ComponentTypes...>& rhs) noexcept {
	// Similar to above
	return !(lhs == rhs);
}

template <typename... ComponentTypes> class sparse_set_view {
	using iterator = sparse_set_view_iterator<ComponentTypes...>;

      public:
	// Please don't touch this, took a long time to figure out
	sparse_set_view(ComponentManager* componentManager) : mComponentManager(componentManager) {
		// This first part makes a vector of all the sizes of the components
		const std::vector<size_t> sizes = {mComponentManager->getPool<ComponentTypes>()->size()...};

		// This second part loops over all the sizes of in the vector and picks out the smallest
		size_t smallestSize = SIZE_T_MAX;
		size_t smallestIndex = 0;

		for (size_t i = 0; i < sizes.size(); ++i) {
			if (sizes[i] <= smallestSize) {
				smallestSize = sizes[i];
				smallestIndex = i;
			}
		}

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

	[[nodiscard]] iterator end() const noexcept { return iterator{mComponentManager, mEntities, mEntities.size()}; }

	/*
	template <typename Type, typename... Other> [[nodiscard]] decltype(auto) get(const entity_type entt) const {
		return get<index_of<Type>, index_of<Other>...>(entt);
	}

	template <std::size_t... Index> [[nodiscard]] decltype(auto) get(const entity_type entt) const {
		if constexpr (sizeof...(Index) == 0) {
			return get(entt, std::index_sequence_for<Get...>{});
		} else if constexpr (sizeof...(Index) == 1) {
			return (storage<Index>()->get(entt), ...);
		} else {
			return std::tuple_cat(storage<Index>()->get_as_tuple(entt)...);
		}
	}
	*/

      private:
	class ComponentManager* mComponentManager;
	std::vector<EntityID> mEntities;
};

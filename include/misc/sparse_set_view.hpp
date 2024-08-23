#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"

#include <algorithm>
#include <span>
#include <vector>

class sparse_set_view_iterator {
      public:
	sparse_set_view_iterator(ComponentManager* componentManager) : mComponentManager(componentManager) {
		(void)mComponentManager;
	}

	/*
	[[nodiscard]] constexpr const EntityID& operator[](const size_t value) const noexcept {
		return (*packed)[mIndex - value];
	}

	[[nodiscard]] constexpr const EntityID& operator*() const noexcept { return operator[](0); }
	*/

      private:
	class ComponentManager* mComponentManager;
};

template <typename... ComponentTypes> class sparse_set_view {
	template <int N, typename... Ts> using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;
	using iterator = sparse_set_view_iterator;

      public:
	// Please don't touch this
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
		// FIXME: This is __NOT__ good
		const std::vector<EntityID*> ids = {mComponentManager->getPool<ComponentTypes>()->data()...};
		for (const auto& id : std::span<EntityID>(ids[smallestIndex], smallestSize)) {
			// This creates a vector of bools for every pool which represent if the pool contains the entity
			const std::vector<bool> poolContains = {mComponentManager->getPool<ComponentTypes>()->contains(id)...};

			// If it does add the entity to our view's pool
			if (std::all_of(poolContains.begin(), poolContains.end(), [](const bool v) { return v; })) {
				mEntities.emplace_back(id);
			}
		}

		// Now at the end of the program, all the entites that are present in all the components are present in mEntities (doesn't count in addition and deletion)
	}

	sparse_set_view(sparse_set_view&&) = delete;
	sparse_set_view(const sparse_set_view&) = delete;
	sparse_set_view& operator=(sparse_set_view&&) = delete;
	sparse_set_view& operator=(const sparse_set_view&) = delete;
	~sparse_set_view() {}

	[[nodiscard]] iterator begin() const noexcept { return iterator{mComponentManager}; }

	[[nodiscard]] iterator end() const noexcept { return iterator{mComponentManager}; }

      private:
	class ComponentManager* mComponentManager;
	std::vector<EntityID> mEntities;
};

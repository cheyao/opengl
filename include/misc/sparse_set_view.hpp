#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"

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
	sparse_set_view(ComponentManager* componentManager) : mComponentManager(componentManager) {
		const std::vector<size_t> s = {mComponentManager->getPool<ComponentTypes>()->size()...};
		const auto pools = std::make_tuple(mComponentManager->getPool<ComponentTypes>()...);
		size_t smallest = SIZE_T_MAX;
		size_t smallestIndex = 0;

		for (size_t i = 0; i < s.size(); ++i) {
			if (s[i] <= smallest) {
				smallest = s[i];
				smallestIndex = i;
			}
		}

		assert(smallest == s[smallestIndex]);

		// Now to create the pool?
		// Make a list of common entities
		for (const auto& entity : mComponentManager->getPool<ComponentTypes>) {
			SDL_Log("Contains: %llu", entity);
		}
		mEntities.emplace_back();
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

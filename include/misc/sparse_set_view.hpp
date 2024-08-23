#pragma once

#include "managers/componentManager.hpp"

#include <vector>

template <typename... ComponentTypes> class sparse_set_view_iterator {
};

template <typename... ComponentTypes> class sparse_set_view {
	using iterator = sparse_set_view_iterator<ComponentTypes...>;

      public:
	sparse_set_view(ComponentManager* manager) : mManager(manager) {
		std::vector<void*> componentIDs[] = {mManager->getID<ComponentTypes>()...};
	}
	sparse_set_view(sparse_set_view&&) = delete;
	sparse_set_view(const sparse_set_view&) = delete;
	sparse_set_view& operator=(sparse_set_view&&) = delete;
	sparse_set_view& operator=(const sparse_set_view&) = delete;
	~sparse_set_view();

	/*
	[[nodiscard]] iterator begin() const noexcept { return iterator{mPackedContainer, mPackedContainer.size()}; }

	[[nodiscard]] iterator end() const noexcept { return iterator{mPackedContainer, {}}; }
*/

      private:
	ComponentManager* mManager;
};

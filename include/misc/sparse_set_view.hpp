#pragma once

#include "managers/componentManager.hpp"

#include <vector>

template <typename... ComponentTypes> class sparse_set_view_iterator {
      public:
	sparse_set_view_iterator(ComponentManager* componentManager) : mComponentManager(componentManager) {
		std::vector<size_t> s = {mComponentManager->getPool<ComponentTypes>()->size()...};
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
	using iterator = sparse_set_view_iterator<ComponentTypes...>;

      public:
	explicit sparse_set_view(ComponentManager* componentManager) : mComponentManager(componentManager) {}
	sparse_set_view(sparse_set_view&&) = delete;
	sparse_set_view(const sparse_set_view&) = delete;
	sparse_set_view& operator=(sparse_set_view&&) = delete;
	sparse_set_view& operator=(const sparse_set_view&) = delete;
	~sparse_set_view();

	[[nodiscard]] iterator begin() const noexcept { return iterator{mComponentManager}; }

	[[nodiscard]] iterator end() const noexcept { return iterator{mComponentManager}; }

      private:
	class ComponentManager* mComponentManager;
};

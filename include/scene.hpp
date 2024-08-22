#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"

#include <memory>
#include <vector>

/*
template <typename Type, typename... Other, typename... Exclude>
[[nodiscard]] basic_view<get_t<storage_for_type<const Type>, storage_for_type<const Other>...>,
			 exclude_t<storage_for_type<const Exclude>...>>
view(exclude_t<Exclude...> = exclude_t{}) const {
	const auto cpools =
		std::make_tuple(assure<std::remove_const_t<Type>>(), assure<std::remove_const_t<Other>>()...,
				assure<std::remove_const_t<Exclude>>()...);
	basic_view<get_t<storage_for_type<const Type>, storage_for_type<const Other>...>,
		   exclude_t<storage_for_type<const Exclude>...>>
		elem{};
	std::apply([&elem](const auto*... curr) { ((curr ? elem.storage(*curr) : void()), ...); }, cpools);
	return elem;
}
*/

class Scene {
      public:
	Scene();
	Scene(Scene&&) = delete;
	Scene(const Scene&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;
	~Scene();

	[[nodiscard]] EntityID newEntity();
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->emplace(entity, args...);
	}

	template <typename Component> Component& get(const EntityID entity) {
		return static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->get(entity);
	}

	template <typename Component> sparse_set<Component>& getView() {
		return *static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>());
	}

      private:
	std::unique_ptr<class EntityManager> mEntityManager;
	std::unique_ptr<class ComponentManager> mComponentManager;
	std::vector<EntityID> mEntities;
};

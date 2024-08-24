#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"
#include "misc/sparse_set_view.hpp"

#include <vector>

class Scene {
      public:
	Scene() : mEntityManager(new EntityManager()), mComponentManager(new ComponentManager()) {}
	Scene(Scene&&) = delete;
	Scene(const Scene&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;
	~Scene() {
		delete mEntityManager;
		delete mComponentManager;
	}

	// This returns a UUID for a new entity
	[[nodiscard]] EntityID newEntity() {
		mEntities.emplace_back(mEntityManager->getEntity());
		return mEntities.back();
	}

	// Adds a component to an entity
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->emplace(entity, args...);
	}

	// TODO:
	// void remove()

	// Returns the component of the entity
	template <typename Component> [[nodiscard]] Component& get(const EntityID entity) const {
		return static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->get(entity);
	}

	// Returns a view of the components
	template <typename... Components> [[nodiscard]] sparse_set_view<Components...> view() const {
		return sparse_set_view<Components...>(mComponentManager);
	}

      private:
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;
	std::vector<EntityID> mEntities;
};

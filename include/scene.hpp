#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"

#include <memory>
#include <vector>

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
		static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())
			->emplace_back(entity, args...);
	}

	template <typename Component> Component& get(const EntityID entity) {
		return static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->get(entity);
	}

      private:
	std::unique_ptr<class EntityManager> mEntityManager;
	std::unique_ptr<class ComponentManager> mComponentManager;
	std::vector<EntityID> mEntities;
};

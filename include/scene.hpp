#pragma once

#include "managers/entityManager.hpp"

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

      private:
	std::unique_ptr<class EntityManager> mEntityManager;
	std::unique_ptr<class ComponentManager> mComponentManager;
	std::vector<Entity> mEntities;
};

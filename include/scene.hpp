#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class Scene {
      public:
	Scene();
	Scene(Scene&&) = delete;
	Scene(const Scene&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;
	~Scene();

      private:
	std::unique_ptr<EntityManager> mEntityManager;
	std::unordered_map<ComponentID, sparse_set<std::uint64_t>> mPool;
};


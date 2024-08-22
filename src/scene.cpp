#include "scene.hpp"

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"

#include <memory>

Scene::Scene()
	: mEntityManager(std::make_unique<EntityManager>()), mComponentManager(std::make_unique<ComponentManager>()) {}

Scene::~Scene() {}

EntityID Scene::newEntity() { return mEntities.back(); }

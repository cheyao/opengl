#include "scene.hpp"

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"

#include <memory>

Scene::Scene()
	: mEntityManager(std::make_unique<EntityManager>()), mComponentManager(std::make_unique<ComponentManager>()) {}

Scene::~Scene() {}

EntityID Scene::newEntity() {
	mEntities.emplace_back(mEntityManager->getEntity());
	return mEntities.back().id;
}

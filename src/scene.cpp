#include "scene.hpp"

#include "managers/entityManager.hpp"

#include <memory>

Scene::Scene() : mEntityManager(std::make_unique<EntityManager>()) {}

Scene::~Scene() {}

#include "managers/managerManager.hpp"

#include "game.hpp"
#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"

ManagerManager::ManagerManager(Game* game) : mGame(game) {
	mSystemManager = new SystemManager(mGame);
	mEntityManager = new EntityManager();
	mComponentManager = new ComponentManager();
}

ManagerManager::~ManagerManager() {
	delete mSystemManager;
	delete mEntityManager;
	delete mComponentManager;
}

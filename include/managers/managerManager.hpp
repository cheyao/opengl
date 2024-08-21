#pragma once

#include "managers/entityManager.hpp"
class ManagerManager {
      public:
	explicit ManagerManager(class Game* game);
	ManagerManager(ManagerManager&&) = default;
	ManagerManager(const ManagerManager&) = default;
	ManagerManager& operator=(ManagerManager&&) = default;
	ManagerManager& operator=(const ManagerManager&) = default;
	~ManagerManager();

	class SystemManager* getSystemManager() const { return mSystemManager; };
	class EntityManager* getEntityManager() const { return mEntityManager; };
	class ComponentManager* getComponentManager() const { return mComponentManager; };

      private:
	class Game* mGame;

	class SystemManager* mSystemManager;
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;
};

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

      private:
	class Game* mGame;

	class SystemManager* mSystemManager;
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;
};

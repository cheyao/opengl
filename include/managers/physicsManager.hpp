#pragma once

#include <vector>

class PhysicsManager {
      public:
	PhysicsManager() = default;
	PhysicsManager(PhysicsManager&&) = delete;
	PhysicsManager(const PhysicsManager&) = delete;
	PhysicsManager& operator=(PhysicsManager&&) = delete;
	PhysicsManager& operator=(const PhysicsManager&) = delete;
	~PhysicsManager() = default;

	void addCollisionComponent(class CollisionComponent* component);

      private:
	std::vector<class CollisionComponent*> mCollisionComponents;
};

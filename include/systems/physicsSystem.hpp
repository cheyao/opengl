#pragma once

#include "systems/system.hpp"

#include <vector>

class PhysicsManager : public System {
      public:
	PhysicsManager() = default;
	PhysicsManager(PhysicsManager&&) = delete;
	PhysicsManager(const PhysicsManager&) = delete;
	PhysicsManager& operator=(PhysicsManager&&) = delete;
	PhysicsManager& operator=(const PhysicsManager&) = delete;
	~PhysicsManager() = default;

	void collide();
	void addCollisionComponent(class CollisionComponent* component);

      private:
	// Collision tests
	bool collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b);

	std::vector<class CollisionComponent*> mCollisionComponents;
};

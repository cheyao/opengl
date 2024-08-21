#pragma once

#include <vector>

class PhysicsSystem {
      public:
	PhysicsSystem();
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	~PhysicsSystem() = default;

	void collide();
	void addCollisionComponent(class CollisionComponent* component);

      private:
	// Collision tests
	bool collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b);

	std::vector<class CollisionComponent*> mCollisionComponents;
};

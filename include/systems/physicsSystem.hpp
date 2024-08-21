#pragma once

class PhysicsSystem {
      public:
	PhysicsSystem();
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	~PhysicsSystem() = default;

	void collide();

      private:
	// Collision tests
	bool collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b);
};

#pragma once

class PhysicsSystem {
      public:
	explicit PhysicsSystem(class Game* game);
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	~PhysicsSystem() = default;

	void update(class Scene* scene, const float delta);
	void collide();

      private:
	// Collision tests
	bool collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b);

	class Game* mGame;
};

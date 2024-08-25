#pragma once

#include "managers/entityManager.hpp"

class PhysicsSystem {
      public:
	explicit PhysicsSystem(class Game* game);
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	~PhysicsSystem() = default;

	void update(class Scene* scene, const float delta);
	void collide(class Scene* scene);

      private:
	// Collision tests
	bool AABBxAABB(class Scene* scene, const EntityID a, const EntityID b);

	class Game* mGame;
};

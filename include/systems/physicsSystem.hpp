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
	bool AABBxAABB(const class Scene* scene, const EntityID entity, const EntityID block) const;
	bool collidingBellow(const class Scene* scene, const EntityID entity, const EntityID block) const;
	void pushBack(class Scene* scene, const EntityID entity, EntityID block);
	void markDirty(class Scene* scene);

	class Game* mGame;
};

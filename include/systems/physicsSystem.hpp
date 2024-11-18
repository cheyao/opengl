#pragma once

#include "managers/entityManager.hpp"
#include "opengl/shader.hpp"

#include <cstdint>
#include <unordered_map>

class PhysicsSystem {
      public:
	constexpr const static inline std::uint64_t PHYSICS_DIRTY_SIGNAL = "physics_dirty"_u;

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
	void itemPhysics(class Scene* scene);

	class Game* mGame;

	// Collision cache
	struct {
		std::unordered_map<EntityID, EntityID> lastAbove;
	} mCache;
};

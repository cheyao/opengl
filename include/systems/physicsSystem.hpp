#pragma once

#include "managers/entityManager.hpp"
#include "opengl/shader.hpp"

#include <cstdint>
#include <unordered_map>

class PhysicsSystem {
      public:
	constexpr const static inline std::uint64_t PHYSICS_DIRTY_SIGNAL = "physics_dirty"_u;

	explicit PhysicsSystem() noexcept;
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	~PhysicsSystem() = default;

	void update(class Scene* scene, const float delta);
	void collide(class Scene* scene);

      private:
	constexpr const static inline std::uint64_t PICK_UP_RANGE = 150;
	constexpr const static inline std::uint64_t PICK_UP_RANGE_SQ = PICK_UP_RANGE * PICK_UP_RANGE;

	// Collision tests
	bool AABBxAABB(const class Scene* scene, const EntityID entity, const EntityID block) const;
	bool collidingBellow(const class Scene* scene, const EntityID entity, const EntityID block) const;
	void pushBack(class Scene* scene, const EntityID entity, EntityID block);
	// Manages the falling and picking of items
	void itemPhysics(class Scene* scene);

	class Game* mGame;

	// Collision cache
	struct {
		std::unordered_map<EntityID, EntityID> lastAbove;
		std::array<std::array<EntityID, 16 * 3>, 128> chunk;
	} mCache;
};

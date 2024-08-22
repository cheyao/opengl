#pragma once

#include <cstdint>
#include <deque>
#include <limits>

using EntityID = std::uint64_t;

constexpr const static EntityID MAX_ENTITIES = std::numeric_limits<EntityID>::max();

class EntityManager {
      public:
	EntityManager();
	EntityManager(EntityManager&&) = delete;
	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(EntityManager&&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;
	~EntityManager() = default;

	EntityID getEntity();
	void releaseEntity(EntityID entity);
	EntityID getEntityCount() const { return mNext - mReleased.size(); }

      private:
	EntityID mNext;
	std::deque<EntityID> mReleased;
};

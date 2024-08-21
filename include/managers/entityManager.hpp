#pragma once

#include <cstdint>
#include <deque>
#include <limits>
#include <vector>
#include <bitset>

using ComponentID = std::uint64_t;
using EntityID = std::uint64_t;

constexpr const static ComponentID MAX_COMPONENTS = std::numeric_limits<ComponentID>::max();
constexpr const static std::uint64_t MAX_COMPONENT_TYPES = 64;

constexpr const static EntityID MAX_ENTITIES = std::numeric_limits<EntityID>::max();

using ComponentMask = std::bitset<MAX_COMPONENT_TYPES>;

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

	std::vector<ComponentMask> mEntities;
};

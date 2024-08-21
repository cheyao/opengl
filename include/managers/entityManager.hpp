#pragma once

#include <cstdint>
#include <deque>
#include <limits>

constexpr const static std::uint64_t MAX_ENTITIES = std::numeric_limits<std::uint64_t>::max();

class EntityManager {
      public:
	EntityManager();
	EntityManager(EntityManager&&) = delete;
	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(EntityManager&&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;
	~EntityManager() = default;

	std::uint64_t getEntity();
	std::uint64_t getEntityCount() const { return mNext - mReleased.size(); }

      private:
	std::uint64_t mNext;

	std::deque<std::uint64_t> mReleased;
};

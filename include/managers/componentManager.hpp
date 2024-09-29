#pragma once

#include "misc/sparse_set.hpp"

#include <cassert>
#include <cstdint>
#include <limits>

using ComponentID = std::uint64_t;

constexpr const static ComponentID MAX_COMPONENTS = std::numeric_limits<ComponentID>::max();

class ComponentManager {
      public:
	ComponentManager() : mComponentCount(0) {}
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
	~ComponentManager() {
		for (const auto& pool : mPools) {
			delete pool;
		}
	}

	template <typename Component> [[nodiscard]] ComponentID getID() noexcept {
		assert(mComponentCount < MAX_COMPONENTS);
		static size_t componentID = mComponentCount++;

		// Maybe getPool()?
		if (mPools.size() <= componentID) {
			mPools.emplace_back(static_cast<base_sparse_set*>(new sparse_set<Component>()));
		}

		return componentID;
	}

	template <typename Component> [[nodiscard]] sparse_set<Component>* getPool() {
		return dynamic_cast<sparse_set<Component>*>(mPools[getID<Component>()]);
	}

	void erase(const EntityID entity) noexcept {
		for (auto& pool : mPools) {
			if (pool->contains(entity)) {
				pool->erase(entity);
			}
		}
	}

      private:
	size_t mComponentCount;
	std::vector<base_sparse_set*> mPools;
	// References to destructors of the pools
};

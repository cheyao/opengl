#pragma once

#include "misc/sparse_set.hpp"

#include <cassert>
#include <cstdint>
#include <limits>

using ComponentID = std::uint64_t;
constexpr const static ComponentID MAX_COMPONENTS = std::numeric_limits<ComponentID>::max();

class ComponentManager {
	ComponentManager() = default;
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;

      public:
	~ComponentManager() {}

	static ComponentManager* getInstance() {
		static ComponentManager manager;
		return &manager;
	}

	template <typename Component> [[nodiscard]] utils::sparse_set<Component>* getPool() {
		static utils::sparse_set<Component> pool = (mPools.emplace_back(&pool), utils::sparse_set<Component>());

		return &pool;
	}

	void erase(const EntityID entity) noexcept {
		for (auto* pool : mPools) {
			if (pool->contains(entity)) {
				pool->erase(entity);
			}
		}
	}

	void clear() noexcept {
		for (auto* pool : mPools) {
			pool->clear();
		}
	}

      private:
	std::vector<utils::sparse_set_interface*> mPools;
};

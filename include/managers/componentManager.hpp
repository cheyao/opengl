#pragma once

#include "misc/sparse_set.hpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <typeindex>
#include <unordered_map>

using ComponentID = std::uint64_t;
constexpr const static ComponentID MAX_COMPONENTS = std::numeric_limits<ComponentID>::max();

class ComponentManager {
      public:
	ComponentManager() = default;
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
	~ComponentManager() {
		for (const auto& [_, pool] : mPools) {
			delete pool;
		}
	}

	template <typename Component> [[nodiscard]] utils::sparse_set<Component>* getPool() {
		[[unlikely]] if (!mPools.contains(typeid(Component))) {
			mPools[typeid(Component)] = new utils::sparse_set<Component>();
		}

		return static_cast<utils::sparse_set<Component>*>(mPools[typeid(Component)]);
	}

	void erase(const EntityID entity) noexcept {
		for (auto& [_, pool] : mPools) {
			if (pool->contains(entity)) {
				pool->erase(entity);
			}
		}
	}

      private:
	std::unordered_map<std::type_index, utils::sparse_set_interface*> mPools;
};

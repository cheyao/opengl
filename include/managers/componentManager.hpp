#pragma once

#include "misc/sparse_set.hpp"
#include <cstdint>
#include <limits>

using ComponentID = std::uint64_t;

constexpr const static ComponentID MAX_COMPONENTS = std::numeric_limits<ComponentID>::max();

class ComponentManager {
      public:
	ComponentManager();
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
	~ComponentManager() = default;

	template <typename Component> [[nodiscard]] ComponentID getID();
	template <typename Component> [[nodiscard]] sparse_set<Component>* getPool();

      private:
	size_t mComponentCount;
	std::vector<void*> mPools;
};

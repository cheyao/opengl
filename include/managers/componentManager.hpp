#pragma once

#include <cstdint>
#include <limits>

using ComponentID = std::uint64_t;

constexpr const static ComponentID MAX_COMPONENTS = std::numeric_limits<ComponentID>::max();
constexpr const static std::uint64_t MAX_COMPONENT_TYPES = 64;

class ComponentManager {
      public:
	ComponentManager();
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
	~ComponentManager() = default;

	template <typename T> ComponentID getID();

      private:
	int mComponentCount;
};

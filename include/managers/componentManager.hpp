#pragma once

#include "misc/sparse_set.hpp"

#include <cassert>
#include <cstdint>
#include <limits>

#ifdef DEBUG
#include <SDL3/SDL.h>
#define LOG SDL_Log
#else
#define LOG (void)
#endif

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

	template <typename Component> [[nodiscard]] ComponentID getID() {
		assert(mComponentCount < MAX_COMPONENTS);
		static size_t componentID = mComponentCount++;

		if (mPools.size() <= componentID) {
			mPools.emplace_back(new sparse_set<Component>());
			LOG("New pool: %zu", (uintptr_t)mPools.back());
		}

		LOG("New comp: %zu", componentID);

		return componentID;
	}

	template <typename Component> [[nodiscard]] sparse_set<Component>* getPool() {
		return static_cast<sparse_set<Component>*>(mPools[getID<Component>()]);
	}

      private:
	size_t mComponentCount;
	std::vector<void*> mPools;
};

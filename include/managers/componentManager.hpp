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
		// std::apply([] {}, mComponentTypes);
	}

	template <typename Component> [[nodiscard]] ComponentID getID() noexcept {
		assert(mComponentCount < MAX_COMPONENTS);
		static size_t componentID = mComponentCount++;

		// Maybe getPool()?
		if (mPools.size() <= componentID) {
			mPools.emplace_back(new sparse_set<Component>());
			// mComponentTypes = std::tuple_cat(mComponentTypes, std::make_tuple<Component>());
		}

		return componentID;
	}

	template <typename Component> [[nodiscard]] sparse_set<Component>* getPool() {
		return static_cast<sparse_set<Component>*>(mPools[getID<Component>()]);
	}

      private:
	size_t mComponentCount;
	std::vector<void*> mPools;
};

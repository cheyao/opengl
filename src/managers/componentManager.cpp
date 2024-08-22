#include "managers/componentManager.hpp"
#include "misc/sparse_set.hpp"

#include <cassert>

ComponentManager::ComponentManager() : mComponentCount(0) {}

template <typename Component> ComponentID ComponentManager::getID() {
	assert(mComponentCount < MAX_COMPONENTS);
	static size_t componentID = mComponentCount++;

	if (mPools.size() < componentID) {
		mPools.emplace_back(new sparse_set<Component>());
	}

	return componentID;
}

template <typename Component> [[nodiscard]] sparse_set<Component>* ComponentManager::getPool() {
	return mPools[getID<Component>()];
}

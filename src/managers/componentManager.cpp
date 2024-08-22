#include "managers/componentManager.hpp"

#include <cassert>

ComponentManager::ComponentManager() : mComponentCount(0) {}

template <typename T> ComponentID ComponentManager::getID() {
	assert(mComponentCount < MAX_COMPONENTS);
	static int componentID = mComponentCount++;
	return componentID;
}


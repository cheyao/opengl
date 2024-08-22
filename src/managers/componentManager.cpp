#include "managers/componentManager.hpp"

ComponentManager::ComponentManager() : mComponentCount(0) {}

template <typename T> ComponentID ComponentManager::getID() {
	static int componentID = mComponentCount++;
	return componentID;
}


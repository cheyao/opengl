#include "managers/physicsManager.hpp"

#include "components/collisionComponent.hpp"

void PhysicsManager::addCollisionComponent(class CollisionComponent* component) {
	const int priority = component->getCollisionPriority();
	auto iter = mCollisionComponents.begin();

	for (; iter != mCollisionComponents.end(); ++iter) {
		if (priority < (*iter)->getCollisionPriority()) {
			break;
		}
	}

	mCollisionComponents.insert(iter, component);
}

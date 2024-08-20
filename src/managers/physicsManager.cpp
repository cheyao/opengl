#include "managers/physicsManager.hpp"

#include "actors/actor.hpp"
#include "components/collisionComponent.hpp"
#include "components/recatangleCollisionComponent.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>

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

void PhysicsManager::collide() {
	// PERF: There surely is a better way to do this

	for (size_t a = 0; a < mCollisionComponents.size(); ++a) {
		enum { RECTANGLE, CIRCLE } aType;

		if (dynamic_cast<RectangleCollisionComponent*>(mCollisionComponents[a]) != nullptr) {
			aType = RECTANGLE;
		} else {
			SDL_LogError(SDL_LOG_PRIORITY_ERROR,
				     "\x1B[31mPhysics error: Didn't find type for object\033[0m");

			// Error box?
		}

		for (size_t b = a + 1; b < mCollisionComponents.size(); ++b) {
			// Test collision

			if (aType == RECTANGLE &&
			    dynamic_cast<RectangleCollisionComponent*>(mCollisionComponents[b]) != nullptr) {
				if (collideRectRect(
					    dynamic_cast<RectangleCollisionComponent*>(mCollisionComponents[a]),
					    dynamic_cast<RectangleCollisionComponent*>(mCollisionComponents[b]))) {
					mCollisionComponents[a]->collide(mCollisionComponents[b]);
					mCollisionComponents[b]->collide(mCollisionComponents[a]);
				}
			} else {
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mPhysics error: Didn't find type for "
								     "object or collision function\033[0m");
			}
		}
	}
}

// TODO: https://noonat.github.io/intersect/#aabb-vs-swept-aabb
bool PhysicsManager::collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b) {
	Eigen::Vector2f aa = a->getOwner()->getPosition().cast<Eigen::Vector2f>();
	Eigen::Vector2f ba = b->getOwner()->getPosition().cast<Eigen::Vector2f>();
	Eigen::Vector2f ab = aa + a->getSize();
	Eigen::Vector2f bb = ba + a->getSize();

	bool no = ab.x() < ba.x() || ab.y() < ba.y() || bb.x() < aa.x() || bb.y() < aa.y();

	return !no;
}

#include "managers/physicsManager.hpp"

#include "actors/actor.hpp"
#include "components/collisionComponent.hpp"
#include "components/rectangleCollisionComponent.hpp"
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

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

// TODO: https://noonat.github.io/intersect/#aabb-vs-swept-aabb
bool PhysicsManager::collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b) {
	assert(a->getOwner()->getPosition().z() == 0 && "Not 2D");
	assert(a->getOwner()->getPosition().z() == 0 && "Not 2D");

	Eigen::Vector2f aa = a->getOwner()->getPosition().head<2>();
	Eigen::Vector2f ba = b->getOwner()->getPosition().head<2>();
	Eigen::Vector2f ab = aa + a->getSize();
	Eigen::Vector2f bb = ba + a->getSize();

	// If one of these four are true, it means the cubes are not intersecting
	bool notIntercecting = ab.x() < ba.x()	   // Amax to the left of Bmin
			       || ab.y() < ba.y()  // Amax to the bottom of Bmin
			       || bb.x() < aa.x()  // Bmax to the left of Amax
			       || bb.y() < aa.y(); // Bmax to the bottom of Amin

	return !notIntercecting;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

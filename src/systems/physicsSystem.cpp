#include "systems/physicsSystem.hpp"

#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>

void PhysicsSystem::collide() {
	return;
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

/*
// TODO: https://noonat.github.io/intersect/#aabb-vs-swept-aabb
bool PhysicsSystem::collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b) {
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
*/

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

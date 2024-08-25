#include "systems/physicsSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "scene.hpp"

#include <SDL3/SDL.h>

PhysicsSystem::PhysicsSystem(Game* game) : mGame(game) {}

void PhysicsSystem::update(Scene* scene, float delta) {
	for (const auto& entity : scene->view<Components::position, Components::velocity>()) {
		scene->get<Components::position>() += velocity.vel * delta;

		SDL_Log("vel %f %f", position.pos.x(), velocity.vel.y());
	}
}

void PhysicsSystem::collide() { return; }

/*
bool PhysicsSystem::collideRectRect(class RectangleCollisionComponent* a, class RectangleCollisionComponent* b) {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

	assert(a->getOwner()->getPosition().z() == 0 && "Not 2D");
	assert(a->getOwner()->getPosition().z() == 0 && "Not 2D");

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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

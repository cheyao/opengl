#include "components/physicsComponent.hpp"

#include "actors/actor.hpp"
#include "components/component.hpp"
#include "third_party/Eigen/Core"

PhysicsComponent::PhysicsComponent(Actor* owner, int priority) : Component(owner, priority), mVelocity(Eigen::Vector3f::Identity()) {
}
void PhysicsComponent::update(float delta) {
	Eigen::Vector3f pos = mOwner->getPosition();

	pos += mVelocity * delta;

	if (pos.y() < 0) {
		pos.y() = 0;
	}

	mOwner->setPosition(pos);
}

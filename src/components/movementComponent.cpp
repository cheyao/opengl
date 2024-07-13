#include "components/movementComponent.hpp"

#include "actors/actor.hpp"
#include "components/component.hpp"
#include "third_party/Eigen/src/Geometry/Quaternion.h"
#include "utils.hpp"

MovementComponent::MovementComponent(class Actor* owner, int updateOrder)
	: Component(owner, updateOrder), mAngularSpeed(0.0f), mForwardSpeed(0.0f) {}

void MovementComponent::update(float delta) {
	if (!nearZero(mAngularSpeed)) {
		Eigen::Quaternionf rot = mOwner->getRotation();
		float angle = mAngularSpeed * delta;
		Eigen::Quaternionf inc(Eigen::AngleAxisf(angle, Eigen::Vector3f::UnitY()));
		Eigen::Quaternionf out = rot * inc;
		mOwner->setRotation(out.normalized());
	}

	if (!nearZero(mForwardSpeed)  || !nearZero(mStrafeSpeed) ) {
		Eigen::Vector3f pos = mOwner->getPosition();
		pos += mOwner->getForward() * mForwardSpeed * delta;
		pos += mOwner->getRight() * mStrafeSpeed * delta;
		mOwner->setPosition(pos);
	}
}

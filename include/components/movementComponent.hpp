#pragma once

#include "components/component.hpp"

class MovementComponent : public Component {
  public:
	explicit MovementComponent(class Actor* owner, int updateOrder = 10);
	MovementComponent(MovementComponent&&) = default;
	MovementComponent(const MovementComponent&) = default;
	MovementComponent& operator=(MovementComponent&&) = default;
	MovementComponent& operator=(const MovementComponent&) = default;

	void update(float delta) override;
	
	float getAngularSpeed() const { return mAngularSpeed; }
	void setAngularSpeed(const float speed) { mAngularSpeed = speed; }

	float getForwardSpeed() const { return mForwardSpeed; }
	void setForwardSpeed(const float speed) { mForwardSpeed = speed; }

	float getStrafeSpeed() const { return mStrafeSpeed; }
	void setStrafeSpeed(const float speed) { mStrafeSpeed = speed; }

  private:
	float mAngularSpeed;
	float mForwardSpeed;
	float mStrafeSpeed;
};


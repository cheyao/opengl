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
	
	float GetAngularSpeed() const { return mAngularSpeed; }
	void setAngularSpeed(float speed) { mAngularSpeed = speed; }

	float GetForwardSpeed() const { return mForwardSpeed; }
	void setForwardSpeed(float speed) { mForwardSpeed = speed; }

  private:
	float mAngularSpeed;
	float mForwardSpeed;
};


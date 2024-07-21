#pragma once

#include "components/component.hpp"

class MovementComponent : public Component {
  public:
	explicit MovementComponent(class Actor* owner, int updateOrder = 10);
	MovementComponent(MovementComponent&&) = delete;
	MovementComponent(const MovementComponent&) = delete;
	MovementComponent& operator=(MovementComponent&&) = delete;
	MovementComponent& operator=(const MovementComponent&) = delete;
	~MovementComponent() override = default;

	void update(float delta) override;
	
	[[nodiscard]] float getAngularSpeed() const { return mAngularSpeed; }
	void setAngularSpeed(const float speed) { mAngularSpeed = speed; }

	[[nodiscard]] float getForwardSpeed() const { return mForwardSpeed; }
	void setForwardSpeed(const float speed) { mForwardSpeed = speed; }

	[[nodiscard]] float getStrafeSpeed() const { return mStrafeSpeed; }
	void setStrafeSpeed(const float speed) { mStrafeSpeed = speed; }

  private:
	float mAngularSpeed;
	float mForwardSpeed;
	float mStrafeSpeed;
};


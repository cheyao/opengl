#pragma once

#include "components/component.hpp"
#include "third_party/Eigen/Core"

class PhysicsComponent : public Component {
      public:
	explicit PhysicsComponent(class Actor* owner, int priority = 100);
	PhysicsComponent(PhysicsComponent&&) = delete;
	PhysicsComponent(const PhysicsComponent&) = delete;
	PhysicsComponent& operator=(PhysicsComponent&&) = delete;
	PhysicsComponent& operator=(const PhysicsComponent&) = delete;
	~PhysicsComponent() override = default;

	void addVelocity(const Eigen::Vector3f velocity) { mVelocity += velocity; }

	void update(float delta) override;

      private:
	Eigen::Vector3f mVelocity;
};

#pragma once

#include "components/collisionComponent.hpp"
#include "third_party/Eigen/Core"

class RectangleCollisionComponent : public CollisionComponent {
      public:
	explicit RectangleCollisionComponent(class Actor* owner, Eigen::Vector2f size, int updatePriority = 100);
	RectangleCollisionComponent(RectangleCollisionComponent&&) = delete;
	RectangleCollisionComponent(const RectangleCollisionComponent&) = delete;
	RectangleCollisionComponent& operator=(RectangleCollisionComponent&&) = delete;
	RectangleCollisionComponent& operator=(const RectangleCollisionComponent&) = delete;
	~RectangleCollisionComponent() = default;

      private:
	Eigen::Vector2f mSize;
};

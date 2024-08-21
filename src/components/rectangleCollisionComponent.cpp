#include "components/rectangleCollisionComponent.hpp"

#include "actors/actor.hpp"
#include "components/collisionComponent.hpp"
#include "third_party/Eigen/Core"

RectangleCollisionComponent::RectangleCollisionComponent(Actor* owner, Eigen::Vector2f size, int updatePriority)
						    : CollisionComponent(owner, updatePriority), mSize(size) {}


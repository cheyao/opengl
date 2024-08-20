#include "components/collisionComponent.hpp"

#include "actors/actor.hpp"
#include "components/component.hpp"
#include "game.hpp"
#include "managers/physicsManager.hpp"

CollisionComponent::CollisionComponent(class Actor* owner, int updatePriority)
	: Component(owner, 50), mCollisionPriority(updatePriority) {
	mOwner->getGame()->getPhysicsManager()->addCollisionComponent(this);
}

#include "components/component.hpp"

#include "actors/actor.hpp"

Component::Component(Actor* owner, int priority) : mOwner(owner), mUpdatePriority(priority) {
	mOwner->addComponent(this);
}

Component::~Component() { mOwner->removeComponent(this); }

void Component::update([[maybe_unused]] float delta) {}

void Component::input([[maybe_unused]] const bool* keystate) {}

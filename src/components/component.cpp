#include "components/component.hpp"

#include "actors/actor.hpp"

#include <cstdint>

Component::Component(Actor* owner, int priority) : mOwner(owner), mUpdatePriority(priority) {
	mOwner->addComponent(this);
}

Component::~Component() { mOwner->removeComponent(this); }

void Component::update(float delta) { (void)delta; }

void Component::input(const uint8_t* keystate) { (void)keystate; }

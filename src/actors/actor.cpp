#include "actors/actor.hpp"

#include "components/component.hpp"
#include "game.hpp"

#include <algorithm>
#include <cstdint>

Actor::Actor(Game* game)
	: mState(ALIVE), mPosition(Eigen::Vector3f::Zero()), mRotation(Eigen::Quaternionf::Identity()), mScale(1.0f),
	  mGame(game) {
	mGame->addActor(this);
}

Actor::~Actor() {
	mGame->removeActor(this);

	while (!mComponents.empty()) {
		delete mComponents.back();
	}
}

void Actor::update(float delta) {
	if (mState != ALIVE) {
		return;
	}

	updateComponents(delta);
	updateActor(delta);
}

void Actor::updateComponents(float delta) {
	for (const auto& component : mComponents) {
		component->update(delta);
	}
}

void Actor::updateActor(float delta) { (void)delta; }

void Actor::input(const uint8_t* keystate) {
	[[unlikely]] if (mState != ALIVE) { return; }

	for (const auto& component : mComponents) {
		component->input(keystate);
	}

	actorInput(keystate);
}

void Actor::actorInput(const uint8_t* keystate) { (void)keystate; }

void Actor::addComponent(Component* component) {
	const int priority = component->getUpdatePriority();
	auto iter = mComponents.begin();

	for (; iter != mComponents.end(); ++iter) {
		if (priority < (*iter)->getUpdatePriority()) {
			break;
		}
	}

	mComponents.insert(iter, component);
}

void Actor::removeComponent(Component* component) {
	const auto& iter = std::find(mComponents.begin(), mComponents.end(), component);

	[[likely]] if (iter != mComponents.end()) { mComponents.erase(iter); }
}

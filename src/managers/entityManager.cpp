#include "managers/entityManager.hpp"

EntityManager::EntityManager() : mNext(0) {}

EntityID EntityManager::getEntity() {
	if (!mReleased.empty()) {
		EntityID first = mReleased.front();
		mReleased.pop_front();
		return first;
	}

	return mNext++;
}

void EntityManager::releaseEntity(EntityID entity) { mReleased.emplace_back(entity); }

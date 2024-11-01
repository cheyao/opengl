#include "managers/entityManager.hpp"

#include <algorithm>

EntityManager::EntityManager() : mNext(1) {}

EntityID EntityManager::getEntity() {
	if (!mReleased.empty()) {
		EntityID first = mReleased.front();
		mReleased.pop_front();
		return first;
	}

	return mNext++;
}

void EntityManager::releaseEntity(EntityID entity) { mReleased.emplace_back(entity); }

bool EntityManager::valid(const EntityID entity) const {
	return entity != 0 && std::find(mReleased.begin(), mReleased.end(), entity) == mReleased.end() &&
	       entity < mNext;
}

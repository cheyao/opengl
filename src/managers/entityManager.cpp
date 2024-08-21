#include "managers/entityManager.hpp"

EntityManager::EntityManager() : mNext(0) {}

std::uint64_t EntityManager::getEntity() {
	if (!mReleased.empty()) {
		std::uint64_t first = mReleased.front();
		mReleased.pop_front();
		return first;
	}

	return mNext++;
}

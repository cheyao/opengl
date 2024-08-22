#include "misc/sparse_set.hpp"

template <typename Component> sparse_set<Component>::sparse_set() {
	mSparseContainer.reserve(64);
	mPackedContainer.reserve(64);
}

template <typename Component>
template <typename... Args>
void sparse_set<Component>::emplace(const EntityID entity, Args&&... args) {
	mSparseContainer[entity] = mPackedContainer.size();
	mPackedContainer.emplace_back(entity);
	mComponents.emplace_back(args...);
}

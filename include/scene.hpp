#pragma once
#include "components.hpp"
#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set_view.hpp"

#include <SDL3/SDL_assert.h>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

// Cache is defined in utils.cpp
extern std::vector<int*> mViewCache;

template <typename... Components>
[[nodiscard]] utils::sparse_set_view<Components...> view_impl(ComponentManager* const mComponentManager) {
	static int key = (mViewCache.push_back(&key), false);
	static utils::sparse_set_view<Components...> view = utils::sparse_set_view<Components...>(mComponentManager);

	if (key) {
		view = utils::sparse_set_view<Components...>(mComponentManager);
		key = false;
	}

	return view;
}

class Scene {
#ifdef IMGUI
	// Needed for signal list menu
	friend class SystemManager;
#endif
      public:
	Scene() : mEntityManager(new EntityManager()), mComponentManager(new ComponentManager()), mSignals() {}

	Scene(Scene&&) = delete;
	Scene(const Scene&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;

	~Scene() {
		delete mEntityManager;
		delete mComponentManager;

		markAllCachesDirty();
	}

	// This returns a UUID for a new entity
	[[nodiscard]] EntityID newEntity() {
		const EntityID entity = mEntityManager->getEntity();
		markAllCachesDirty();
		return entity;
	}

	// Adds a component to an entity
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		mComponentManager->getPool<Component>()->emplace(entity, std::forward<Args>(args)...);
		// Mark entire view cache as dirty because we changed the pools
		markAllCachesDirty();
	}

	template <typename Component> [[nodiscard]] Component& get(const EntityID entity) const {
		return mComponentManager->getPool<Component>()->get(entity);
	}

	template <typename Component> [[nodiscard]] bool contains(const EntityID entity) const {
		return mComponentManager->getPool<Component>()->contains(entity);
	}

	template <typename... Components> [[nodiscard]] utils::sparse_set_view<Components...> view() {
		return ::view_impl<Components...>(mComponentManager);
	}

	// Remove an entity
	void erase(const EntityID entity) noexcept {
		SDL_assert(entity != 0);
		mComponentManager->erase(entity);
		mEntityManager->releaseEntity(entity);

		markAllCachesDirty();
	}

	[[nodiscard]] bool valid(const EntityID entity) noexcept { return mEntityManager->valid(entity); }

	[[nodiscard]] std::int64_t& getSignal(const std::uint64_t signal) noexcept {
		if (!mSignals.contains(signal)) {
			mSignals[signal] = false;
		}
		return mSignals[signal];
	}

	void clearSignals() noexcept { mSignals.clear(); }

	// A structure used e.g. for the mouse
	struct {
		Components::Item item = static_cast<Components::Item>(0);
		std::uint64_t count = 0;
	} mMouse;

      private:
	// Mark entire view cache as dirty, because any change in the pools invalidates all cached views
	void markAllCachesDirty() {
		for (int* const cachePtr : mViewCache) {
			*cachePtr = true;
		}
	}

	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;

	std::unordered_map<std::uint64_t, std::int64_t> mSignals;
};

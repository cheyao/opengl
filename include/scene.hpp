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

template <typename... Components> [[nodiscard]] utils::sparse_set_view<Components...> view_impl() {
	static int key = (mViewCache.push_back(&key), false);
	static utils::sparse_set_view<Components...> view = utils::sparse_set_view<Components...>();

	if (key) {
		view = utils::sparse_set_view<Components...>();
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
	Scene() noexcept : mEntityManager(new EntityManager()), mSignals() {}

	Scene(Scene&&) = delete;
	Scene(const Scene&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;

	~Scene() noexcept {
		delete mEntityManager;
		markAllCachesDirty();
	}

	// This returns a UUID for a new entity
	[[nodiscard]] EntityID newEntity() noexcept {
		const EntityID entity = mEntityManager->getEntity();
		markAllCachesDirty();
		return entity;
	}

	// Adds a component to an entity
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		static auto* const pool = ComponentManager::getInstance()->getPool<Component>();

		pool->emplace(entity, std::forward<Args>(args)...);
		// Mark entire view cache as dirty because we changed the pools
		markAllCachesDirty();
	}

	template <typename Component> [[nodiscard]] Component& get(const EntityID entity) const {
		return ComponentManager::getInstance()->getPool<Component>()->get(entity);
	}

	template <typename Component> [[nodiscard]] bool contains(const EntityID entity) const {
		return ComponentManager::getInstance()->getPool<Component>()->contains(entity);
	}

	template <typename... Components> [[nodiscard]] utils::sparse_set_view<Components...> view() {
		return ::view_impl<Components...>();
	}

	// Remove an entity
	void erase(const EntityID entity) noexcept {
		SDL_assert(entity != 0);
		ComponentManager::getInstance()->erase(entity);
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

	std::unordered_map<std::uint64_t, std::int64_t> mSignals;
};

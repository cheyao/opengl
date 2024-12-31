#pragma once

#include "components.hpp"
#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set_view.hpp"

#include <SDL3/SDL_assert.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

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
	}

	// This returns a UUID for a new entity
	[[nodiscard]] EntityID newEntity() {
		const EntityID entity = mEntityManager->getEntity();

		return entity;
	}
	// Adds a component to an entity
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		mComponentManager->getPool<Component>()->emplace(entity, args...);
	}

	// Returns the component of the entity
	template <typename Component> [[nodiscard]] Component& get(const EntityID entity) const {
		return mComponentManager->getPool<Component>()->get(entity);
	}

	// Returns weather a entity contains a component
	template <typename Component> [[nodiscard]] bool contains(const EntityID entity) const {
		return mComponentManager->getPool<Component>()->contains(entity);
	}

	// Returns a view of the components
	template <typename... Components> [[nodiscard]] utils::sparse_set_view<Components...> view() {
		return utils::sparse_set_view<Components...>(mComponentManager);
	}

	void erase(const EntityID entity) noexcept {
		SDL_assert(entity != 0);

		mComponentManager->erase(entity);
		mEntityManager->releaseEntity(entity);
	}

	[[nodiscard]] bool valid(const EntityID entity) noexcept { return mEntityManager->valid(entity); }

	[[nodiscard]] std::int64_t& getSignal(const std::uint64_t signal) noexcept {
		if (!mSignals.contains(signal)) {
			mSignals[signal] = false;
		}

		return mSignals[signal];
	}
	void clearSignals() noexcept { mSignals.clear(); }

	struct {
		Components::Item item = static_cast<Components::Item>(0);
		std::size_t count = 0;
	} mMouse;

      private:
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;

	std::unordered_map<std::uint64_t, std::int64_t> mSignals;
};

#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"
#include "misc/sparse_set_view.hpp"

#include <string>
#include <unordered_map>
#include <vector>

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
#ifdef IMGUI
		mDebugEntities.emplace_back(entity);
#endif
		return entity;
	}
	// Adds a component to an entity
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->emplace(entity, args...);
	}

	// Returns the component of the entity
	template <typename Component> [[nodiscard]] Component& get(const EntityID entity) const {
		return static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->get(entity);
	}

	// Returns weather a entity contains a component
	template <typename Component> [[nodiscard]] bool contains(const EntityID entity) const {
		return static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->contains(entity);
	}

	// Returns a view of the components
	template <typename... Components> [[nodiscard]] sparse_set_view<Components...> view() const {
		return sparse_set_view<Components...>(mComponentManager);
	}

	void erase(const EntityID entity) noexcept {
		mComponentManager->erase(entity);
		mEntityManager->releaseEntity(entity);

#ifdef IMGUI
		mDebugEntities.erase(std::find(mDebugEntities.begin(), mDebugEntities.end(), entity));
#endif
	}

	[[nodiscard]] bool valid(const EntityID entity) noexcept { return mEntityManager->valid(entity); }

	[[nodiscard]] bool& getSignal(const std::string& signal) noexcept {
		if (!mSignals.contains(signal)) {
			mSignals[signal] = false;
		}

		return mSignals[signal];
	}
	void clearSignals() noexcept { mSignals.clear(); }

      private:
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;

	std::unordered_map<std::string, bool> mSignals;
#ifdef IMGUI
	std::vector<EntityID> mDebugEntities;
#endif
};

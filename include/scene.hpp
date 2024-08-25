#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"
#include "misc/sparse_set_view.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class Scene {
      public:
	Scene() : mEntityManager(new EntityManager()), mComponentManager(new ComponentManager()) {}
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
		mEntities.emplace_back(mEntityManager->getEntity());
		return mEntities.back();
	}

	// Adds a component to an entity
	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->emplace(entity, args...);
	}

	// TODO:
	// void remove()

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

	bool& getSignal(const std::string& signal) {
		if (!mSignals.contains(signal)) {
			mSignals[signal] = false;
		}

		return mSignals[signal];
	}
	void clearSignals() { mSignals.clear(); }

      private:
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;
	std::vector<EntityID> mEntities;
	std::unordered_map<std::string, bool> mSignals;
};

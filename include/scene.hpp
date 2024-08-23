#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"
#include "misc/sparse_set_view.hpp"

#include <memory>
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

	[[nodiscard]] EntityID newEntity() {
		mEntities.emplace_back(mEntityManager->getEntity());
		return mEntities.back();
	}

	template <typename Component, typename... Args> void emplace(const EntityID entity, Args&&... args) {
		static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->emplace(entity, args...);
	}

	template <typename Component> Component& get(const EntityID entity) const {
		return static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>())->get(entity);
	}

	/*
	template <typename Component> sparse_set<Component>& view() const {
		return *static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>());
	}
	*/

	// TODO: Exclude
	template <typename... Components> [[nodiscard]] sparse_set_view<Components...> view() const {
		// Make a tuple of all the pools of components passed into the vaargs
		/*
		const auto cpools = std::make_tuple(
			static_cast<sparse_set<Components>*>(mComponentManager->getPool<Components>())...);

		sparse_set_view<Components...> view{};

		std::apply([&view](const auto*... curr) { ((curr ? view.storage(*curr) : void()), ...); }, cpools);
		*/

		return sparse_set_view<Components...>(mComponentManager);
	}

      private:
	class EntityManager* mEntityManager;
	class ComponentManager* mComponentManager;
	std::vector<EntityID> mEntities;
};

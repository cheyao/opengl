#pragma once

#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set.hpp"
#include "misc/sparse_set_view.hpp"

#include <memory>
#include <vector>

class Scene {
	template <typename... ComponentTypes> friend class sparse_set_view;

      public:
	Scene()
		: mEntityManager(std::make_unique<EntityManager>()),
		  mComponentManager(std::make_unique<ComponentManager>()) {}
	Scene(Scene&&) = delete;
	Scene(const Scene&) = delete;
	Scene& operator=(Scene&&) = delete;
	Scene& operator=(const Scene&) = delete;
	~Scene() {}

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

	template <typename Component> sparse_set<Component>& view() const {
		return *static_cast<sparse_set<Component>*>(mComponentManager->getPool<Component>());
	}

	/*
	// TODO: Exclude
	template <typename... Components> [[nodiscard]] sparse_set_view<sparse_set<const Components>...> view() const {
		// Make a tuple of all the pools of components passed into the vaargs
		const auto cpools = std::make_tuple(
			static_cast<sparse_set<Components>*>(mComponentManager->getPool<Components>())...);

		// std::apply([&elem](const auto*... curr) { ((curr ? elem.storage(*curr) : void()), ...); }, cpools);

		sparse_set_view<sparse_set<Components>...> elem{};

		return elem;
	}
	*/

      private:
	std::unique_ptr<class EntityManager> mEntityManager;
	std::unique_ptr<class ComponentManager> mComponentManager;
	std::vector<EntityID> mEntities;
};

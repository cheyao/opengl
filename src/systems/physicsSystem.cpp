#include "systems/physicsSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "imgui.h"
#include "managers/entityManager.hpp"
#include "scene.hpp"

#include <SDL3/SDL.h>

PhysicsSystem::PhysicsSystem(Game* game) : mGame(game) {}

void PhysicsSystem::update(Scene* scene, float delta) {
	for (const auto& entity : scene->view<Components::position, Components::velocity>()) {
		scene->get<Components::position>(entity).pos += scene->get<Components::velocity>(entity).vel * delta;
	}
}

// FIXME: Use some nice trees https://gamedev.stackexchange.com/questions/26501/how-does-a-collision-engine-work
void PhysicsSystem::collide(Scene* scene) {
	// Get a list of all the entities we need to check
	auto entities = std::vector<EntityID>();
	scene->view<Components::collision, Components::position>().each(
		[&entities](const EntityID& entity) { entities.emplace_back(entity); });

	// Debug editor
#if defined(IMGUI) && defined(DEBUG)
	static bool editor = false;
	ImGui::Begin("Main menu");
	ImGui::Checkbox("Collision box editor", &editor);
	ImGui::End();

	if (editor) {
		ImGui::Begin("Collision editor");
		ImGui::End();
	}

	for (const auto& entity : entities) {
		(void)entity;
		// TODO: editor
	}
#endif

	for (size_t i = 0; i < entities.size(); ++i) {
		for (size_t j = i + 1; j < entities.size(); ++j) {
			if (AABBxAABB(scene, entities[i], entities[j]) {
				SDL_Log("Collide");
			}
		}
	}

	return;
}

bool PhysicsSystem::AABBxAABB(Scene* scene, const EntityID a, const EntityID b) {
	const Eigen::Vector2f& aa = scene->get<Components::position>(a).pos;
	const Eigen::Vector2f& ba = scene->get<Components::position>(b).pos;
	const Eigen::Vector2f ab = aa + scene->get<Components::collision>(a).size;
	const Eigen::Vector2f bb = ba + scene->get<Components::collision>(b).size;

	// If one of these four are true, it means the cubes are not intersecting
	bool notIntercecting = ab.x() < ba.x()	   // Amax to the left of Bmin
			       || ab.y() < ba.y()  // Amax to the bottom of Bmin
			       || bb.x() < aa.x()  // Bmax to the left of Amax
			       || bb.y() < aa.y(); // Bmax to the bottom of Amin

	return !notIntercecting;
}

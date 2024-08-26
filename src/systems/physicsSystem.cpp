#include "systems/physicsSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "scene.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cassert>
#include <format>

#ifdef IMGUI
#include "imgui.h"
#endif

// The physicsSystem is in charge of the collision and mouvements
PhysicsSystem::PhysicsSystem(Game* game) : mGame(game) {}

void PhysicsSystem::update(Scene* scene, float delta) {
	for (const auto& entity : scene->view<Components::position, Components::velocity>()) {
		scene->get<Components::position>(entity).pos += scene->get<Components::velocity>(entity).vel * delta;
	}
}

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
		scene->getSignal("collisionEditor") = true;

		ImGui::Begin("Collision editor");

		for (const auto& entity : entities) {
			if (ImGui::TreeNode(std::format("Entity {}", entity).data())) {
				ImGui::SliderFloat2(std::format("Position for entity {}", entity).data(),
						    scene->get<Components::position>(entity).pos.data(), 0.0f,
						    std::max(mGame->getDemensions().x(), mGame->getDemensions().y()));

				ImGui::SliderFloat2(std::format("Offset for entity {}", entity).data(),
						    scene->get<Components::collision>(entity).offset.data(), -500,
						    +500);

				ImGui::SliderFloat2(std::format("Size for entity {}", entity).data(),
						    scene->get<Components::collision>(entity).size.data(), 0.0f,
						    std::max(mGame->getDemensions().x(), mGame->getDemensions().y()));

				ImGui::TreePop();
			}
			// TODO: editor
		}

		ImGui::End();
	}
#endif

	// Iterate over all pairs of colliders
	// PERF: Use some nice trees https://gamedev.stackexchange.com/questions/26501/how-does-a-collision-engine-work
	for (size_t i = 0; i < entities.size(); ++i) {
		for (size_t j = i + 1; j < entities.size(); ++j) {
			if (AABBxAABB(scene, entities[i], entities[j])) {
				pushBack(scene, entities[i], entities[j]);
			}
		}
	}

	return;
}

bool PhysicsSystem::AABBxAABB(const Scene* scene, const EntityID a, const EntityID b) const {
	assert(a != b);

	const Eigen::Vector2f& leftA =
		scene->get<Components::position>(a).pos + scene->get<Components::collision>(a).offset;
	const Eigen::Vector2f& rightA = leftA + scene->get<Components::collision>(a).size;

	const Eigen::Vector2f& leftB =
		scene->get<Components::position>(b).pos + scene->get<Components::collision>(b).offset;
	const Eigen::Vector2f& rightB = leftB + scene->get<Components::collision>(b).size;

	// assert(!std::isnan(aa.x()) && !std::isinf(aa.x()) && aa.x() >= 0);
	// assert(!std::isnan(ab.x()) && !std::isinf(ab.x()) && ab.x() >= 0);
	// assert(!std::isnan(ba.x()) && !std::isinf(ba.x()) && ba.x() >= 0);
	// assert(!std::isnan(bb.x()) && !std::isinf(bb.x()) && bb.x() >= 0);

	// If one of these four are true, it means the cubes are not intersecting
	const bool notIntercecting = rightA.x() < leftB.x()	// Amax to the left of Bmin
				     || rightA.y() < leftB.y()	// Amax to the bottom of Bmin
				     || rightB.x() < leftA.x()	// Bmax to the left of Amax
				     || rightB.y() < leftA.y(); // Bmax to the bottom of Amin

	return !notIntercecting;
}

// TODO: Read
// https://gamedev.stackexchange.com/questions/38891/making-an-efficient-collision-detection-system/38893#38893
// TODO: Read
// https://gamedev.stackexchange.com/questions/38613/how-do-i-detect-collision-between-movie-clips/38635#38635
void PhysicsSystem::pushBack(class Scene* scene, const EntityID a, EntityID b) {
	assert(a != b);
	// Two components cannot be stationary at the same time
	assert((!scene->get<Components::collision>(a).stationary || !scene->get<Components::collision>(a).stationary) &&
	       "Hey! I can't resolve the collision of two stationary objects!");

	// Thx stack https://gamedev.stackexchange.com/questions/18302/2d-platformer-collisions
	// See https://github.com/MonoGame/MonoGame.Samples/blob/3.8.2/Platformer2D/Platformer2D.Core/Game/Player.cs
	// To
	// https://github.com/MonoGame/MonoGame.Samples/blob/3.8.2/Platformer2D/Platformer2D.Core/Game/RectangleExtensions.cs#L30

	const Eigen::Vector2f& leftA =
		scene->get<Components::position>(a).pos + scene->get<Components::collision>(a).offset;
	const Eigen::Vector2f& centerA = leftA + scene->get<Components::collision>(a).size / 2;

	const Eigen::Vector2f& leftB =
		scene->get<Components::position>(b).pos + scene->get<Components::collision>(b).offset;
	const Eigen::Vector2f& centerB = leftB + scene->get<Components::collision>(b).size / 2;

	const Eigen::Vector2f& distance = centerA - centerB;
	const Eigen::Vector2f& minDistance =
		(scene->get<Components::collision>(a).size + scene->get<Components::collision>(b).size) / 2;

	assert(!(std::abs(distance.x()) > minDistance.x() || std::abs(distance.y()) > minDistance.y()) &&
	       "The objects are not colliding?");

	const Eigen::Vector2f& depth = minDistance - distance;

	if (depth.x() < depth.y()) {
		scene->get<Components::position>(a).pos.x() += (depth / 2).x();
		scene->get<Components::position>(b).pos.x() += (-depth / 2).x();
	} else {
		scene->get<Components::position>(a).pos.y() += (depth / 2).y();
		scene->get<Components::position>(b).pos.y() += (-depth / 2).y();
	}
	/*
	float depthX = distance.x() > 0 ? minDistanceX - distanceX : -minDistanceX - distanceX;
	float depthY = distance.y() > 0 ? minDistanceY - distanceY : -minDistanceY - distanceY;
	*/
}

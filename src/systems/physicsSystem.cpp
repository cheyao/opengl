#include "systems/physicsSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"

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
	constexpr const static float G = 15.0f;
	constexpr const static float jumpForce = 500.0f;

	auto entities = std::vector<EntityID>();
	scene->view<Components::collision, Components::position>().each(
		[&entities](const EntityID& entity) { entities.emplace_back(entity); });

	// FIXME: This is messed up when the framerate isn't constant
	// https://gamedev.stackexchange.com/questions/15708/how-can-i-implement-gravity
	// To fix it we need to add a acceleration vector (Wow school knowladge has usages)
	for (const auto& entity : scene->view<Components::position, Components::velocity>()) {
		bool onGround = false;
		for (size_t i = 0; i < entities.size(); ++i) {
			if (entity != entities[i] && collidingBellow(scene, entity, entities[i])) {
				onGround = true;

				break;
			}
		}

		if (onGround) {
			if (scene->contains<Components::misc>(entity) &&
			    scene->get<Components::misc>(entity).what == Components::misc::JUMP &&
			    (mGame->getKeystate()[SDL_SCANCODE_UP] == true ||
			     mGame->getKeystate()[SDL_SCANCODE_SPACE] == true)) {
				scene->get<Components::velocity>(entity).vel.y() = jumpForce;
			} else {
				scene->get<Components::velocity>(entity).vel.y() = 0.0f;
			}
		} else {
			scene->get<Components::velocity>(entity).vel.y() -= G;
		}

		scene->get<Components::position>(entity).pos += scene->get<Components::velocity>(entity).vel * delta;
		scene->get<Components::velocity>(entity).vel.x() =
			scene->get<Components::velocity>(entity).vel.x() * 0.7;
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
	assert(a != b && "Hey! Why are the same objects colliding into each other");

	const Eigen::Vector2f& leftA =
		scene->get<Components::position>(a).pos + scene->get<Components::collision>(a).offset;
	const Eigen::Vector2f& rightA = leftA + scene->get<Components::collision>(a).size;

	const Eigen::Vector2f& leftB =
		scene->get<Components::position>(b).pos + scene->get<Components::collision>(b).offset;
	const Eigen::Vector2f& rightB = leftB + scene->get<Components::collision>(b).size;

	assert(!std::isnan(leftA.x()) && !std::isinf(leftA.x()));
	assert(!std::isnan(rightA.x()) && !std::isinf(rightA.x()));
	assert(!std::isnan(leftB.x()) && !std::isinf(leftB.x()));
	assert(!std::isnan(rightB.x()) && !std::isinf(rightB.x()));
	assert(!std::isnan(leftA.y()) && !std::isinf(leftA.y()));
	assert(!std::isnan(rightA.y()) && !std::isinf(rightA.y()));
	assert(!std::isnan(leftB.y()) && !std::isinf(leftB.y()));
	assert(!std::isnan(rightB.y()) && !std::isinf(rightB.y()));

	// If one of these four are true, it means the cubes are not intersecting
	const bool notIntercecting = rightA.x() <= leftB.x()	 // Amax to the left of Bmin
				     || rightA.y() <= leftB.y()	 // Amax to the bottom of Bmin
				     || rightB.x() <= leftA.x()	 // Bmax to the left of Amax
				     || rightB.y() <= leftA.y(); // Bmax to the bottom of Amin

	// So return the inverse of not intersecting
	return !notIntercecting;
}

bool PhysicsSystem::collidingBellow(const class Scene* scene, const EntityID main, const EntityID b) const {
	assert(main != b && "Hey! Why are the same objects colliding into each other");

	const Eigen::Vector2f& leftA =
		scene->get<Components::position>(main).pos + scene->get<Components::collision>(main).offset;
	const Eigen::Vector2f& rightA = leftA + scene->get<Components::collision>(main).size;

	const Eigen::Vector2f& leftB =
		scene->get<Components::position>(b).pos + scene->get<Components::collision>(b).offset;
	const Eigen::Vector2f& rightB = leftB + scene->get<Components::collision>(b).size;

	// on a x level
	const bool notIntercecting = rightA.x() <= leftB.x()	// Amax to the left of Bmin
				     || rightB.x() <= leftA.x() // Bmax to the left of Amax
				     || rightA.y() <= leftB.y();

	if (notIntercecting) {
		return false;
	}

	// Remove one dot five to be more precise
	if ((leftA.y() - 1.5f) < rightB.y()) {
		return true;
	}

	return false;
}

// TODO: Read
// https://gamedev.stackexchange.com/questions/38891/making-an-efficient-collision-detection-system/38893#38893
// TODO: Read
// https://gamedev.stackexchange.com/questions/38613/how-do-i-detect-collision-between-movie-clips/38635#38635
// Corners: https://gamedev.stackexchange.com/questions/17502/how-to-deal-with-corner-collisions-in-2d?noredirect=1&lq=1
// https://gamedev.stackexchange.com/questions/29371/how-do-i-prevent-my-platformers-character-from-clipping-on-wall-tiles?noredirect=1&lq=1
/*
 * Here we first calculate the positions of the squares
 * Then we calculate the center of the cubes from the positions
 * After that we calculate the distance between the two center of cubes
 * We then assert that the objects are collising, if not that means `AABBxAABB` has problems
 * Right after we get the depth of the X axis and Y axis that are overlapping
 * Thus we can determine which side we are colliding on by running `std::min(X, Y)`
 * From this info, we will be able to calculate the ammount we need to push back
 * Which has two scenarios:
 * 1. One object is static, so find the one that isn't calculate neg or pos, and push
 * 2. Both aren't static, thus push back both by half the overlap
 * (If the objects are both stationary, pass)
 */
void PhysicsSystem::pushBack(class Scene* scene, const EntityID a, EntityID b) {
	assert(a != b);
	// Two components cannot be stationary at the same time
	if (scene->get<Components::collision>(a).stationary && scene->get<Components::collision>(b).stationary) {
		return;
	}

	/* Thx stack https://gamedev.stackexchange.com/questions/18302/2d-platformer-collisions
	 * See https://github.com/MonoGame/MonoGame.Samples/blob/3.8.2/Platformer2D/Platformer2D.Core/Game/Player.cs
	 * https://github.com/MonoGame/MonoGame.Samples/blob/3.8.2/Platformer2D/Platformer2D.Core/Game/RectangleExtensions.cs#L30
	 */

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

	const float& depthX = (distance.x() > 0 ? minDistance.x() : -minDistance.x()) - distance.x();
	const float& depthY = (distance.y() > 0 ? minDistance.y() : -minDistance.y()) - distance.y();

	const Eigen::Vector2f& depth = Eigen::Vector2f(depthX, depthY);

	if (scene->get<Components::collision>(a).stationary || scene->get<Components::collision>(b).stationary) {
		const EntityID movable = scene->get<Components::collision>(a).stationary ? b : a;

		// The + and - are relative to the two, so when moving a gotta add, moving b gotta subtract
		// Why a + and b -? IDFK. Just tried out a bunch of possible solutions and got this
		// It works, don't touch (if it's not broken)
		if (std::abs(depth.x()) <= std::abs(depth.y())) {
			scene->get<Components::position>(movable).pos.x() += (movable == b ? -1 : 1) * depth.x();
		} else {
			scene->get<Components::position>(movable).pos.y() += (movable == b ? -1 : 1) * depth.y();
		}
	} else {
		if (std::abs(depth.x()) <= std::abs(depth.y())) {
			scene->get<Components::position>(a).pos.x() += (depth / 2).x();
			scene->get<Components::position>(b).pos.x() += (-depth / 2).x();
		} else {
			scene->get<Components::position>(a).pos.y() += (depth / 2).y();
			scene->get<Components::position>(b).pos.y() += (-depth / 2).y();
		}
	}
}

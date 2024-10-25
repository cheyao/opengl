#include "systems/physicsSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set_view.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

#ifdef IMGUI
#include "imgui.h"

#include <format>
#endif

// The physicsSystem is in charge of the collision and mouvements
PhysicsSystem::PhysicsSystem(Game* game) : mGame(game) {}

/*
 * FIXME:
 * Currently we are checking the player by using a bitmask or smt
 * But it should just be any entity with
 * <Components::position, Components::velocity, Components::force(, Components::move?)>
 * Like this we won't need to depend on a lot of checks and prayers
 */
void PhysicsSystem::update(Scene* scene, float delta) {
	(void)delta;
	constexpr const static float G = 15.0f;
	constexpr const static float jumpForce = 500.0f;

	auto entities = std::vector<EntityID>();
	scene->view<Components::collision, Components::position>().each(
		[&entities](const EntityID& entity) { entities.emplace_back(entity); });

	/*
	 * FIXME: This is messed up when the framerate isn't constant
	 * https://gamedev.stackexchange.com/questions/15708/how-can-i-implement-gravity
	 * To fix it we need to add a acceleration vector(Wow school knowladge has usages)
	 */
	for (const auto& entity : scene->view<Components::position, Components::velocity>()) {
		bool onGround = false;
		for (std::size_t i = 0; i < entities.size(); ++i) {
			if (entity != entities[i] && collidingBellow(scene, entity, entities[i])) {
				onGround = true;

				break;
			}
		}

		if (onGround) {
			if (scene->contains<Components::misc>(entity) &&
			    scene->get<Components::misc>(entity).mWhat & Components::misc::JUMP &&
			    (mGame->getKeystate()[SDL_SCANCODE_UP] == true ||
			     mGame->getKeystate()[SDL_SCANCODE_SPACE] == true)) {
				scene->get<Components::velocity>(entity).mVelocity.y() = jumpForce;
			} else {
				scene->get<Components::velocity>(entity).mVelocity.y() = 0.0f;
			}
		} else {
			scene->get<Components::velocity>(entity).mVelocity.y() -= G;
		}

		scene->get<Components::position>(entity).mPosition += scene->get<Components::velocity>(entity).mVelocity * delta;
		scene->get<Components::velocity>(entity).mVelocity.x() *= 0.7;
	}
}

template <typename T> 
// requires std::ranges::sized_range<T>
requires std::input_or_output_iterator<T>
struct boo {};

void PhysicsSystem::collide(Scene* scene) {
	auto v = scene->view<Components::collision, Components::position>();
	boo<utils::sparse_set_view<Components::collision>::iterator> a;
	// Get a list of all the entities we need to check
	auto entities = std::vector<EntityID>();

	auto blocks = std::vector<EntityID>();
	scene->view<Components::collision, Components::block>().each(
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
						  scene->get<Components::position>(entity).mPosition.data(), 0.0f,
						  SDL_max(mGame->getDemensions().x(), mGame->getDemensions().y()));

				ImGui::SliderFloat2(std::format("Offset for entity {}", entity).data(),
						    scene->get<Components::collision>(entity).mOffset.data(), -500,
						    +500);

				ImGui::SliderFloat2(std::format("Size for entity {}", entity).data(),
						    scene->get<Components::collision>(entity).mSize.data(), 0.0f,
						    SDL_max(mGame->getDemensions().x(), mGame->getDemensions().y()));

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}
#endif

	// Iterate over all pairs of colliders
	// PERF: Use some nice trees https://gamedev.stackexchange.com/questions/26501/how-does-a-collision-engine-work
	// PERF: Multithread
	// FIXME: FIX
	for (const auto& entity : entities) {
		/*
		for (std::size_t j = i + 1; j < blocks.size(); ++j) {
			if (AABBxAABB(scene, entities[i], blocks[j])) {
				pushBack(scene, entities[i], blocks[j]);
			}
		}
		*/
	}

	return;
}

bool PhysicsSystem::AABBxAABB(const Scene* scene, const EntityID a, const EntityID b) const {
	(void)scene;
	(void)a;
	(void)b;
	/*
	SDL_assert(a != b && "Hey! Why are the same objects colliding into each other");

	if (scene->get<Components::collision>(a).mStationary && scene->get<Components::collision>(b).mStationary) {
		return false;
	}

	const Eigen::Vector2f& minA = static_cast<Eigen::Vector2f>(scene->get<Components::position>(a).mPosition) +
				      scene->get<Components::collision>(a).mOffset;
	const Eigen::Vector2f& maxA = minA + scene->get<Components::collision>(a).mSize;

	const Eigen::Vector2f& minB = static_cast<Eigen::Vector2f>(scene->get<Components::position>(b).mPosition) +
				      scene->get<Components::collision>(b).mOffset;
	const Eigen::Vector2f& maxB = minB + scene->get<Components::collision>(b).mSize;

	SDL_assert(!SDL_isnan(minA.x()) && !SDL_isinf(minA.x()));
	SDL_assert(!SDL_isnan(maxA.x()) && !SDL_isinf(maxA.x()));
	SDL_assert(!SDL_isnan(minB.x()) && !SDL_isinf(minB.x()));
	SDL_assert(!SDL_isnan(maxB.x()) && !SDL_isinf(maxB.x()));
	SDL_assert(!SDL_isnan(minA.y()) && !SDL_isinf(minA.y()));
	SDL_assert(!SDL_isnan(maxA.y()) && !SDL_isinf(maxA.y()));
	SDL_assert(!SDL_isnan(minB.y()) && !SDL_isinf(minB.y()));
	SDL_assert(!SDL_isnan(maxB.y()) && !SDL_isinf(maxB.y()));

	// If one of these four are true, it means the cubes are not intersecting
	const bool notIntercecting = maxA.x() <= minB.x()     // Amax to the left of Bmin
				     || maxA.y() <= minB.y()  // Amax to the bottom of Bmin
				     || maxB.x() <= minA.x()  // Bmax to the left of Amax
				     || maxB.y() <= minA.y(); // Bmax to the bottom of Amin

	// So return the inverse of not intersecting
	return !notIntercecting;
	*/
	return false;
}

bool PhysicsSystem::collidingBellow(const class Scene* scene, const EntityID main, const EntityID b) const {
	(void)scene;
	(void)main;
	(void)b;
	return false;
	/*
	SDL_assert(main != b && "Hey! Why are the same objects colliding into each other");

	if (scene->contains<Components::velocity>(main) &&
	    scene->get<Components::velocity>(main).mVelocity.y() > 1.0f) {
		return false;
	}

	const Eigen::Vector2f& minMain =
		static_cast<Eigen::Vector2f>(scene->get<Components::position>(main).mPosition) +
		scene->get<Components::collision>(main).mOffset;
	const Eigen::Vector2f& maxMain = minMain + scene->get<Components::collision>(main).mSize;

	const Eigen::Vector2f& minB = static_cast<Eigen::Vector2f>(scene->get<Components::position>(b).mPosition) +
				      scene->get<Components::collision>(b).mOffset;
	const Eigen::Vector2f& maxB = minB + scene->get<Components::collision>(b).mSize;

	// on a x level
	const bool notIntercecting = maxMain.x() <= minB.x()	// entity to the left of b
				     || maxB.x() <= minMain.x() // b to the left of main
				     || maxMain.y() <= minB.y();

	if (notIntercecting) {
		return false;
	}

	// Remove dot five to be more precise
	if ((minMain.y() - 0.5f) < maxB.y()) {
		return true;
	}

	return false;
	*/
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
 * We then SDL_assert that the objects are collising, if not that means `AABBxAABB` has problems
 * Right after we get the depth of the X axis and Y axis that are overlapping
 * Thus we can determine which side we are colliding on by running `std::min(X, Y)`
 * From this info, we will be able to calculate the ammount we need to push back
 * Which has two scenarios:
 * 1. One object is static, so find the one that isn't calculate neg or pos, and push
 * 2. Both aren't static, thus push back both by half the overlap
 * (If the objects are both stationary, pass)
 */
void PhysicsSystem::pushBack(class Scene* scene, const EntityID a, EntityID b) {
	SDL_assert(a != b);
	// Two components cannot be stationary at the same time
	if (scene->get<Components::collision>(a).mStationary && scene->get<Components::collision>(b).mStationary) {
		return;
	}
	(void)scene;
	(void)a;
	(void)b;

	/*
	 * Thx stack https://gamedev.stackexchange.com/questions/18302/2d-platformer-collisions
	 * See https://github.com/MonoGame/MonoGame.Samples/blob/3.8.2/Platformer2D/Platformer2D.Core/Game/Player.cs
	 * https://github.com/MonoGame/MonoGame.Samples/blob/3.8.2/Platformer2D/Platformer2D.Core/Game/RectangleExtensions.cs#L30
	 */

	/*
	const Eigen::Vector2f& leftA = static_cast<Eigen::Vector2f>(scene->get<Components::position>(a).mPosition) +
				       scene->get<Components::collision>(a).mOffset;
	const Eigen::Vector2f& centerA = leftA + scene->get<Components::collision>(a).mSize / 2;

	const Eigen::Vector2f& leftB = static_cast<Eigen::Vector2f>(scene->get<Components::position>(b).mPosition) +
				       scene->get<Components::collision>(b).mOffset;
	const Eigen::Vector2f& centerB = leftB + scene->get<Components::collision>(b).mSize / 2;

	const Eigen::Vector2f& distance = centerA - centerB;
	const Eigen::Vector2f& minDistance =
		(scene->get<Components::collision>(a).mSize + scene->get<Components::collision>(b).mSize) / 2;

	SDL_assert(!(SDL_abs(distance.x()) > minDistance.x() || SDL_abs(distance.y()) > minDistance.y()) &&
		   "The objects are not colliding?");

	const float& depthX = (distance.x() > 0 ? minDistance.x() : -minDistance.x()) - distance.x();
	const float& depthY = (distance.y() > 0 ? minDistance.y() : -minDistance.y()) - distance.y();

	const Eigen::Vector2f& depth = Eigen::Vector2f(depthX, depthY);

	if (scene->get<Components::collision>(a).mStationary || scene->get<Components::collision>(b).mStationary) {
		const EntityID movable = scene->get<Components::collision>(a).mStationary ? b : a;

		// The + and - are relative to the two, so when moving a gotta add, moving b gotta subtract
		// Why a + and b -? IDFK. Just tried out a bunch of possible solutions and got this
		// It works, don't touch (if it's not broken)
		if (SDL_abs(depth.x()) <= SDL_abs(depth.y())) {
			scene->get<Components::position>(movable).mPosition.x() += (movable == b ? -1 : 1) * depth.x();
		} else {
			scene->get<Components::position>(movable).mPosition.y() += (movable == b ? -1 : 1) * depth.y();
		}
	} else {
		if (SDL_abs(depth.x()) <= SDL_abs(depth.y())) {
			scene->get<Components::position>(a).mPosition.x() += (depth / 2).x();
			scene->get<Components::position>(b).mPosition.x() += (-depth / 2).x();
		} else {
			scene->get<Components::position>(a).mPosition.y() += (depth / 2).y();
			scene->get<Components::position>(b).mPosition.y() += (-depth / 2).y();
		}
	}
	*/
}

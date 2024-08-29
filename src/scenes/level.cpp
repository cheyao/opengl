#include "scenes/level.hpp"

class Texture;

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include <SDL3/SDL.h>
namespace Eigen {
void to_json(nlohmann::json& j, const Vector3f& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}, {"Z", vec.z()}}; }

void from_json(const nlohmann::json& j, Vector3f& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
	j.at("Z").get_to(vec.z());
}

void to_json(nlohmann::json& j, const Vector2f& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}}; }

void from_json(const nlohmann::json& j, Vector2f& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
}
} // namespace Eigen

Level::Level(class Game* game, const std::string& name) : mName(name), mGame(game) {}

void Level::create() {
	SDL_assert(mGame != nullptr);

	mScene = new Scene();
	EntityID player = mScene->newEntity();
	mScene->emplace<Components::texture>(player, mGame->getSystemManager()->getTexture("stone.png", true));
	mScene->emplace<Components::position>(player, Eigen::Vector2f(400.0f, 400.0f));
	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::input>(player, [](class Scene* scene, EntityID entity, const bool* scancodes,
						      [[maybe_unused]] const float delta) {
		Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).vel;

		if (scancodes[SDL_SCANCODE_RIGHT] == true && vel.x() < 220) {
			vel.x() += 70;
		}

		if (scancodes[SDL_SCANCODE_LEFT] == true && vel.x() > -220) {
			vel.x() -= 70;
		}
	});
	mScene->emplace<Components::collision>(
		player, Eigen::Vector2f(0.0f, 0.0f),
		Eigen::Vector2f(mGame->getSystemManager()->getTexture("stone.png", true)->getWidth(),
				mGame->getSystemManager()->getTexture("stone.png", true)->getHeight()));
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);

	EntityID block2 = mScene->newEntity();
	mScene->emplace<Components::texture>(block2, mGame->getSystemManager()->getTexture("stone.png", true));
	mScene->emplace<Components::position>(block2, Eigen::Vector2f(400.0f, 10.0f));
	mScene->emplace<Components::collision>(
		block2, Eigen::Vector2f(0.0f, 0.0f),
		Eigen::Vector2f(mGame->getSystemManager()->getTexture("stone.png", true)->getWidth(),
				mGame->getSystemManager()->getTexture("stone.png", true)->getHeight()),
		true);

	EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));
}

void Level::load([[maybe_unused]] const nlohmann::json data) {
	SDL_assert(data.contains("position"));

	mScene = new Scene();
	EntityID player = mScene->newEntity();
	mScene->emplace<Components::texture>(player, mGame->getSystemManager()->getTexture("stone.png", true));
	mScene->emplace<Components::position>(player, data["position"].get<Eigen::Vector2f>());
	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::input>(player, [](class Scene* scene, EntityID entity, const bool* scancodes,
						      [[maybe_unused]] const float delta) {
		Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).vel;

		if (scancodes[SDL_SCANCODE_RIGHT] == true && vel.x() < 220) {
			vel.x() += 70;
		}

		if (scancodes[SDL_SCANCODE_LEFT] == true && vel.x() > -220) {
			vel.x() -= 70;
		}
	});
	mScene->emplace<Components::collision>(
		player, Eigen::Vector2f(0.0f, 0.0f),
		Eigen::Vector2f(mGame->getSystemManager()->getTexture("stone.png", true)->getWidth(),
				mGame->getSystemManager()->getTexture("stone.png", true)->getHeight()));
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);

	EntityID block2 = mScene->newEntity();
	mScene->emplace<Components::texture>(block2, mGame->getSystemManager()->getTexture("stone.png", true));
	mScene->emplace<Components::position>(block2, Eigen::Vector2f(400.0f, 10.0f));
	mScene->emplace<Components::collision>(
		block2, Eigen::Vector2f(0.0f, 0.0f),
		Eigen::Vector2f(mGame->getSystemManager()->getTexture("stone.png", true)->getWidth(),
				mGame->getSystemManager()->getTexture("stone.png", true)->getHeight()),
		true);

	EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));
}

nlohmann::json Level::save() {
	nlohmann::json data;

	const auto& view = mScene->view<Components::input>();
	SDL_assert(view.size() == 1);

	data["position"] = mScene->get<Components::position>(*view.begin()).pos;

	return data;
}

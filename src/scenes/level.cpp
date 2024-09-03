#include "scenes/level.hpp"

class Texture;

#include "components.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "scenes/chunk.hpp"
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
	constexpr const static float SCALE = 0.7f;
	mScene->emplace<Components::texture>(player, mGame->getSystemManager()->getTexture("stone.png", true), SCALE);
	mScene->emplace<Components::position>(player, Eigen::Vector2f(400.0f, 400.0f));
	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	SDL_Log("B");
	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Out of memory\033[0m");
				break;
		}
	}
	mScene->emplace<Components::input>(player,
					   [](class Scene* scene, EntityID entity, const bool* scancodes, const float) {
						   Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).mVelocity;

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
				mGame->getSystemManager()->getTexture("stone.png", true)->getHeight()) * SCALE);
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);

	EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));

	mChunks.emplace_back(new Chunk(mGame, mScene, 0));
}

void Level::load(const nlohmann::json data) {
	SDL_assert(data.contains("player"));

	mScene = new Scene();
	EntityID player = mScene->newEntity();
	constexpr const static float SCALE = 0.7f;
	mScene->emplace<Components::texture>(player, mGame->getSystemManager()->getTexture("stone.png", true), SCALE);
	mScene->emplace<Components::position>(player, data["player"]["position"].get<Eigen::Vector2f>());
	mScene->emplace<Components::velocity>(player, data["player"]["velocity"]);
	mScene->emplace<Components::input>(player,
					   [](class Scene* scene, EntityID entity, const bool* scancodes, const float) {
						   Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).mVelocity;

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
				mGame->getSystemManager()->getTexture("stone.png", true)->getHeight()) * SCALE);
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);

	EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));

	// SDL_assert(data["chunks"].contains(0));
	mChunks.emplace_back(new Chunk(mGame, mScene, data["chunks"][0]));
}

nlohmann::json Level::save() {
	nlohmann::json data;

	const auto& view = mScene->view<Components::input>();
	SDL_assert(view.size() == 1);

	data["player"]["position"] = mScene->get<Components::position>(*view.begin()).mPosition;
	data["player"]["velocity"] = mScene->get<Components::velocity>(*view.begin()).mVelocity;

	data["chunks"][0] = mChunks.back()->save(mScene);

	return data;
}

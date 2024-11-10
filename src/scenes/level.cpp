#include "scenes/level.hpp"

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
#include <utility>

namespace Eigen {
void to_json(nlohmann::json& j, const Vector3i& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}, {"Z", vec.z()}}; }

void from_json(const nlohmann::json& j, Vector3i& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
	j.at("Z").get_to(vec.z());
}

void to_json(nlohmann::json& j, const Vector2f& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}}; }

void from_json(const nlohmann::json& j, Vector2f& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
}

void to_json(nlohmann::json& j, const Vector2i& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}}; }

void from_json(const nlohmann::json& j, Vector2i& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
}
} // namespace Eigen

Level::Level(class Game* game, const std::string& name)
	: mName(name), mGame(game), mLeft(nullptr), mCenter(nullptr), mRight(nullptr) {}

Level::~Level() { delete mScene; }

void Level::create() {
	SDL_assert(mGame != nullptr);

	mScene = new Scene();

	const auto player = mScene->newEntity();
	mGame->setPlayerID(player);

	createCommon();

	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::position>(
		player, Eigen::Vector2f(0.0f, (Chunk::WATER_LEVEL + 1) * Components::block::BLOCK_SIZE));

	mLeft = new Chunk(mGame, mScene, -1);
	mCenter = new Chunk(mGame, mScene, 0);
	mRight = new Chunk(mGame, mScene, 1);

	mData[CHUNK_KEY]["-"];
	mData[CHUNK_KEY]["+"];
}

void Level::load(const nlohmann::json& data) {
	SDL_assert(data.contains("player"));

	mData = std::move(data);

	mScene = new Scene();
	const EntityID player = mScene->newEntity();
	mGame->setPlayerID(player);

	createCommon();

	mScene->emplace<Components::position>(player, mData[PLAYER_KEY]["position"].template get<Eigen::Vector2f>());
	mScene->emplace<Components::velocity>(player, mData[PLAYER_KEY]["velocity"]);

	const EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));

	auto loadChunk = [this](Chunk*& chunk, const float playerPos) {
		const auto sign = playerPos < 0;
		const auto centerChunk =
			static_cast<int>(playerPos) / Components::block::BLOCK_SIZE / Chunk::CHUNK_WIDTH - sign;

		try {
			chunk = new Chunk(this->mGame, this->mScene,
					  this->mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(centerChunk)]);
		} catch (const nlohmann::json::exception& error) {
			SDL_LogCritical(
				SDL_LOG_CATEGORY_VIDEO,
				"\033[31mGenerating new chunk: Failed to parse json for chunk %d: id %d %s\033[0m",
				centerChunk, error.id, error.what());

			chunk = new Chunk(this->mGame, this->mScene, centerChunk);
		}
	};

	const auto playerPos = mData[PLAYER_KEY]["position"].template get<Eigen::Vector2f>().x();

	loadChunk(mCenter, playerPos);
	loadChunk(mLeft, playerPos - Chunk::CHUNK_WIDTH * Components::block::BLOCK_SIZE + 1);
	loadChunk(mRight, playerPos + Chunk::CHUNK_WIDTH * Components::block::BLOCK_SIZE);
}

nlohmann::json Level::save() {
	const auto playerID = mGame->getPlayerID();

	mData[PLAYER_KEY]["position"] = mScene->get<Components::position>(playerID).mPosition;
	mData[PLAYER_KEY]["velocity"] = mScene->get<Components::velocity>(playerID).mVelocity;

	auto save = [this](Chunk* chunk) {
		this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())] =
			chunk->save(this->mScene);
		delete chunk;
	};

	save(mLeft);
	save(mCenter);
	save(mRight);

	return mData;
}

void Level::update() {
	const auto playerID = mGame->getPlayerID();

	const auto playerX = static_cast<int>(mScene->get<Components::position>(playerID).mPosition.x());
	const auto sign = playerX < 0;
	const auto currentChunk = playerX / Components::block::BLOCK_SIZE / Chunk::CHUNK_WIDTH - sign;

	// Now, we need to check if we need to load a chunk
	if (currentChunk == mCenter->getPosition()) {
		return;
	}

	if (currentChunk == mLeft->getPosition()) {
		mData[CHUNK_KEY][mRight->getPosition() < 0 ? "-" : "+"][SDL_abs(mRight->getPosition())] =
			mRight->save(mScene);
		delete mRight;

		mRight = mCenter;
		mCenter = mLeft;

		try {
			mLeft = new Chunk(mGame, mScene, mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(currentChunk - 1)]);
		} catch (const nlohmann::json::exception& error) {
			SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM,
				    "\033[31mGenerating new chunk: Failed to parse json for chunk %d: id %d %s\033[0m",
				    currentChunk, error.id, error.what());

			mLeft = new Chunk(mGame, mScene, currentChunk - 1);
		}
	} else if (currentChunk == mRight->getPosition()) {
		mData[CHUNK_KEY][mLeft->getPosition() < 0 ? "-" : "+"][SDL_abs(mLeft->getPosition())] =
			mLeft->save(mScene);
		delete mLeft;

		mLeft = mCenter;
		mCenter = mRight;

		try {
			mRight =
				new Chunk(mGame, mScene, mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(currentChunk + 1)]);
		} catch (const nlohmann::json::exception& error) {
			SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM,
				    "\033[31mGenerating new chunk: Failed to parse json for chunk %d: id %d %s\033[0m",
				    currentChunk, error.id, error.what());

			mRight = new Chunk(mGame, mScene, currentChunk + 1);
		}
	} else {
		SDL_Log("\033[33mOut of boundary for chunk %d, loaded chunks: %" PRIu64 " %" PRIu64 " %" PRIu64
			"\033[0m",
			currentChunk, mLeft->getPosition(), mCenter->getPosition(), mRight->getPosition());
		auto save = [this](Chunk* chunk) {
			this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())] =
				chunk->save(this->mScene);

			delete chunk;
		};

		save(mLeft);
		save(mCenter);
		save(mRight);

		mLeft = new Chunk(mGame, mScene, currentChunk - 1);
		mCenter = new Chunk(mGame, mScene, currentChunk);
		mRight = new Chunk(mGame, mScene, currentChunk + 1);
	}
}

void Level::createCommon() {
	const auto player = mGame->getPlayerID();
	auto* playerTexture = mGame->getSystemManager()->getTexture("blocks/stone.png", true);
	mScene->emplace<Components::texture>(player, playerTexture);
	mScene->emplace<Components::collision>(player, Eigen::Vector2f(0.0f, 0.0f), playerTexture->getSize());
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);
	mScene->emplace<Components::input>(
		player, [](class Scene* scene, const EntityID entity, const auto scancodes, const float) {
			Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).mVelocity;

			if (scancodes[SDL_SCANCODE_D] && vel.x() < 220) {
				vel.x() += 70;
			}

			if (scancodes[SDL_SCANCODE_A] && vel.x() > -220) {
				vel.x() -= 70;
			}
		});

	const EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));
}

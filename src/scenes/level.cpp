#include "scenes/level.hpp"

#include "components.hpp"
#include "components/inventory.hpp"
#include "components/noise.hpp"
#include "components/playerInventory.hpp"
#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "scenes/chunk.hpp"
#include "systems/UISystem.hpp"
#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <limits>
#include <utility>

Level::Level(class Game* game, const std::string& name)
	: mName(name), mTextID(0), mLeft(nullptr), mCenter(nullptr), mRight(nullptr), mGame(game),
	  mNoise(new NoiseGenerator()) {}

Level::~Level() { delete mScene; }

void Level::create() {
	SDL_assert(mGame != nullptr);

	mScene = new Scene();

	const auto player = mScene->newEntity();
	mGame->setPlayerID(player);

	createCommon();

	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::position>(
		player, Eigen::Vector2f(0.0f, (Chunk::WATER_LEVEL + 1) * Components::block::BLOCK_SIZE +
						      5 * mNoise->getNoise(0)));
	mScene->emplace<Components::inventory>(player, new PlayerInventory(mGame, 36, player));

	mLeft = new Chunk(mGame, mScene, mNoise.get(), -1);
	mCenter = new Chunk(mGame, mScene, mNoise.get(), 0);
	mRight = new Chunk(mGame, mScene, mNoise.get(), 1);

	mData[CHUNK_KEY]["-"];
	mData[CHUNK_KEY]["+"];
}

void Level::load(const nlohmann::json& data) {
	SDL_assert(data.contains("player"));

	mData = std::move(data);

	mScene = new Scene();
	const EntityID player = mScene->newEntity();
	mGame->setPlayerID(player);

	mNoise->setSeed(mData[CHUNK_KEY]["seed"]);

	createCommon();

	mScene->emplace<Components::position>(player, mData[PLAYER_KEY]["position"].template get<Eigen::Vector2f>());
	mScene->emplace<Components::velocity>(player, mData[PLAYER_KEY]["velocity"]);
	mScene->emplace<Components::inventory>(player,
					       new PlayerInventory(mGame, mData[PLAYER_KEY]["inventory"], player));

	auto loadChunk = [this](Chunk*& chunk, const float playerPos) {
		const auto sign = playerPos < 0;
		const auto centerChunk =
			static_cast<int>(playerPos) / Components::block::BLOCK_SIZE / Chunk::CHUNK_WIDTH - sign;

		if (this->mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(centerChunk)] == nullptr) {
			chunk = new Chunk(this->mGame, this->mScene, mNoise.get(), centerChunk);

			return;
		}

		try {
			chunk = new Chunk(this->mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(centerChunk)], this->mGame,
					  this->mScene);
		} catch (const std::exception& error) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
					"\033[31mGenerating new chunk: Failed to parse json for chunk %d: id %s\033[0m",
					centerChunk, error.what());

			chunk = new Chunk(this->mGame, this->mScene, mNoise.get(), centerChunk);
		}
	};

	const auto playerPos = mData[PLAYER_KEY]["position"].template get<Eigen::Vector2f>().x();
	mScene->mMouse.count = mData[PLAYER_KEY]["mcount"].template get<std::size_t>();
	mScene->mMouse.item = static_cast<Components::Item>(mData[PLAYER_KEY]["mitem"].template get<std::uint64_t>());

	loadChunk(mCenter, playerPos);
	loadChunk(mLeft, playerPos - Chunk::CHUNK_WIDTH * Components::block::BLOCK_SIZE + 0.1);
	loadChunk(mRight, playerPos + Chunk::CHUNK_WIDTH * Components::block::BLOCK_SIZE + 0.1);
}

nlohmann::json Level::save() {
	const auto playerID = mGame->getPlayerID();

	mData[PLAYER_KEY]["position"] = mScene->get<Components::position>(playerID).mPosition;
	mData[PLAYER_KEY]["velocity"] = mScene->get<Components::velocity>(playerID).mVelocity;
	mData[PLAYER_KEY]["inventory"] = mScene->get<Components::inventory>(playerID).mInventory->save();
	mData[PLAYER_KEY]["mcount"] = mScene->mMouse.count;
	mData[PLAYER_KEY]["mitem"] = static_cast<std::uint64_t>(mScene->mMouse.item);
	delete mScene->get<Components::inventory>(playerID).mInventory;
	mData[CHUNK_KEY]["seed"] = mNoise->getSeed();

	auto save = [this](Chunk* chunk) {
		this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())] =
			chunk->save(this->mScene);

		delete chunk;
	};

	save(mLeft);
	save(mCenter);
	save(mRight);

	mScene->erase(playerID);

	return mData;
}

void Level::update(const float delta) {
	mLastTime += delta * 1000;
	if (mLastTime / ROLL_TIME >= 5) {
		mLastTime = 0;
	}

	mScene->get<Components::text>(mTextID).mID = "AD" + std::to_string(mLastTime / ROLL_TIME);

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
			mLeft = new Chunk(mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(currentChunk - 1)], mGame, mScene);
		} catch (const nlohmann::json::exception& error) {
			SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM,
				    "\033[31mGenerating new chunk: Failed to parse json for chunk %d: id %d %s\033[0m",
				    currentChunk, error.id, error.what());

			mLeft = new Chunk(mGame, mScene, mNoise.get(), currentChunk - 1);
		}
	} else if (currentChunk == mRight->getPosition()) {
		mData[CHUNK_KEY][mLeft->getPosition() < 0 ? "-" : "+"][SDL_abs(mLeft->getPosition())] =
			mLeft->save(mScene);
		delete mLeft;

		mLeft = mCenter;
		mCenter = mRight;

		try {
			mRight =
				new Chunk(mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(currentChunk + 1)], mGame, mScene);
		} catch (const nlohmann::json::exception& error) {
			SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM,
				    "\033[31mGenerating new chunk: Failed to parse json for chunk %d: id %d %s\033[0m",
				    currentChunk, error.id, error.what());

			mRight = new Chunk(mGame, mScene, mNoise.get(), currentChunk + 1);
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

		mLeft = new Chunk(mGame, mScene, mNoise.get(), currentChunk - 1);
		mCenter = new Chunk(mGame, mScene, mNoise.get(), currentChunk);
		mRight = new Chunk(mGame, mScene, mNoise.get(), currentChunk + 1);
	}
}

void Level::createCommon() {
	const auto player = mGame->getPlayerID();
	auto* playerTexture = mGame->getSystemManager()->getTexture("blocks/stone.png", true);
	mScene->emplace<Components::texture>(player, playerTexture);
	mScene->emplace<Components::collision>(player, Eigen::Vector2f(0.0f, 0.0f), playerTexture->getSize());
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);
	mScene->emplace<Components::input>(
		player, [this](class Scene* scene, const EntityID entity, const auto scancodes, const float) {
			Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).mVelocity;

			if (scancodes[SDL_SCANCODE_D] && vel.x() < 220) {
				vel.x() += 70;
			}

			if (scancodes[SDL_SCANCODE_A] && vel.x() > -220) {
				vel.x() -= 70;
			}

			// Open inv
			if (scancodes[SDL_SCANCODE_E]) {
				this->mGame->getSystemManager()->getUISystem()->addScreen(
					scene->get<Components::inventory>(entity).mInventory);
			}
		});

	mTextID = mScene->newEntity();
	mScene->emplace<Components::text>(mTextID, "AD0");
	mScene->emplace<Components::position>(mTextID, Eigen::Vector2f(std::numeric_limits<float>::quiet_NaN(),
								       std::numeric_limits<float>::infinity()));
}

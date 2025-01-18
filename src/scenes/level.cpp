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
#include "third_party/rapidjson/allocators.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/rapidjson.h"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>

Level::Level(const std::string& name)
	: mName(name), mTextID(0), mLeft(nullptr), mCenter(nullptr), mRight(nullptr), mGame(Game::getInstance()),
	  mScene(nullptr), mNoise(new NoiseGenerator()) {}

Level::~Level() {
	SDL_Log("Unloading level");
	delete mScene;
}

void Level::create() {
	delete mScene;

	SDL_assert(mGame != nullptr);
	mData.SetObject();

	mScene = new Scene();

	const auto player = mScene->newEntity();
	mGame->setPlayerID(player);

	createCommon();

	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::position>(
		player, Eigen::Vector2f(0.0f, (Chunk::WATER_LEVEL + 1 + (5 * mNoise->getNoise(0))) *
						      Components::block::BLOCK_SIZE));
	mScene->emplace<Components::inventory>(player, new PlayerInventory(mGame, 36));

	mLeft = new Chunk(mScene, mNoise.get(), -1);
	mCenter = new Chunk(mScene, mNoise.get(), 0);
	mRight = new Chunk(mScene, mNoise.get(), 1);

	mData.AddMember(rapidjson::StringRef(CHUNK_KEY), rapidjson::Value(rapidjson::kObjectType),
			mData.GetAllocator());
	mData.AddMember(rapidjson::StringRef(PLAYER_KEY), rapidjson::Value(rapidjson::kObjectType),
			mData.GetAllocator());
	mData[CHUNK_KEY].AddMember("-", rapidjson::Value(rapidjson::kArrayType), mData.GetAllocator());
	mData[CHUNK_KEY].AddMember("+", rapidjson::Value(rapidjson::kArrayType), mData.GetAllocator());

	SDL_assert(mData.HasMember(PLAYER_KEY));
	SDL_assert(mData.HasMember(CHUNK_KEY));
}

void Level::load(rapidjson::Value& data) {
	delete mScene;

	mData.CopyFrom(data, mData.GetAllocator());

	SDL_assert(data.HasMember(PLAYER_KEY));
	SDL_assert(data.HasMember(CHUNK_KEY));

	mScene = new Scene();
	const EntityID player = mScene->newEntity();
	mGame->setPlayerID(player);

	mNoise->setSeed(mData[CHUNK_KEY]["seed"].GetUint64());

	createCommon();

	mScene->emplace<Components::position>(player, getVector2f(mData[PLAYER_KEY]["position"]));
	mScene->emplace<Components::velocity>(player, getVector2f(mData[PLAYER_KEY]["velocity"]));
	mScene->emplace<Components::inventory>(player, new PlayerInventory(mGame, mData[PLAYER_KEY]["inventory"]));

	const auto loadChunk = [this](Chunk*& chunk, const float playerPos) {
		const auto sign = playerPos < 0;
		const auto centerChunk =
			static_cast<int>(playerPos) / Components::block::BLOCK_SIZE / Chunk::CHUNK_WIDTH - sign;

		if (this->mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(centerChunk)].IsNull()) {
			SDL_Log("Chunk %d was Null! Making new chunk", centerChunk);
			chunk = new Chunk(this->mScene, mNoise.get(), centerChunk);

			return;
		}

		chunk = new Chunk(this->mData[CHUNK_KEY][sign ? "-" : "+"][SDL_abs(centerChunk)], this->mScene);
	};

	const auto playerPos = getVector2f(mData[PLAYER_KEY]["position"]).x();
	mScene->mMouse.count = mData[PLAYER_KEY]["mcount"].GetUint64();
	mScene->mMouse.item = static_cast<Components::Item>(mData[PLAYER_KEY]["mitem"].GetUint64());

	loadChunk(mCenter, playerPos);
	loadChunk(mLeft, playerPos - Chunk::CHUNK_WIDTH * Components::block::BLOCK_SIZE + 0.1);
	loadChunk(mRight, playerPos + Chunk::CHUNK_WIDTH * Components::block::BLOCK_SIZE + 0.1);
}

void Level::save(rapidjson::Value& data, rapidjson::MemoryPoolAllocator<>& allocator) {
	SDL_Log("Saving level");
	const auto playerID = mGame->getPlayerID();

	if (!mData[PLAYER_KEY].HasMember("position")) {
		mData[PLAYER_KEY].AddMember(
			"position",
			fromVector2f(mScene->get<Components::position>(playerID).mPosition, mData.GetAllocator())
				.Move(),
			mData.GetAllocator());
		mData[PLAYER_KEY].AddMember(
			"velocity",
			fromVector2f(mScene->get<Components::velocity>(playerID).mVelocity, mData.GetAllocator())
				.Move(),
			mData.GetAllocator());
		mData[PLAYER_KEY].AddMember("inventory", rapidjson::Value(rapidjson::kObjectType).Move(),
					    mData.GetAllocator());
		mScene->get<Components::inventory>(playerID).mInventory->save(mData[PLAYER_KEY]["inventory"],
									      mData.GetAllocator());

		mData[PLAYER_KEY].AddMember("mcount", mScene->mMouse.count, mData.GetAllocator());
		mData[PLAYER_KEY].AddMember("mitem", static_cast<std::uint64_t>(mScene->mMouse.item),
					    mData.GetAllocator());
	} else {
		mData[PLAYER_KEY]["position"] =
			fromVector2f(mScene->get<Components::position>(playerID).mPosition, mData.GetAllocator())
				.Move();
		mData[PLAYER_KEY]["velocity"] =
			fromVector2f(mScene->get<Components::velocity>(playerID).mVelocity, mData.GetAllocator())
				.Move();
		mData[PLAYER_KEY]["inventory"] = rapidjson::Value(rapidjson::kObjectType).Move();
		mScene->get<Components::inventory>(playerID).mInventory->save(mData[PLAYER_KEY]["inventory"],
									      mData.GetAllocator());

		mData[PLAYER_KEY]["mcount"] = mScene->mMouse.count;
		mData[PLAYER_KEY]["mitem"] = static_cast<std::uint64_t>(mScene->mMouse.item);
	}

	SDL_assert(mData.HasMember(CHUNK_KEY));
	if (mData[CHUNK_KEY].HasMember("seed")) {
		mData[CHUNK_KEY]["seed"] = mNoise->getSeed();
	} else {
		mData[CHUNK_KEY].AddMember("seed", mNoise->getSeed(), mData.GetAllocator());
	}

	delete mScene->get<Components::inventory>(playerID).mInventory;

	mScene->erase(playerID);

	const auto save = [this](Chunk* chunk) {
		SDL_assert(chunk != nullptr);

		while (this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"].Size() <=
		       std::llabs(chunk->getPosition())) {
			this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"].PushBack(
				rapidjson::Value(rapidjson::kArrayType).SetObject().Move(), this->mData.GetAllocator());
		}

		this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())].SetObject();

		chunk->save(this->mScene,
			    this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())],
			    this->mData.GetAllocator());

		delete chunk;
	};

	save(mLeft);
	save(mCenter);
	save(mRight);

	data.CopyFrom(mData.Move(), allocator);
}

void Level::update(const float delta) {
	// TODO: Use unique ptr
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

	const auto pad = [this](std::int64_t position) {
		while (this->mData[CHUNK_KEY][position < 0 ? "-" : "+"].Size() <= std::llabs(position)) {
			this->mData[CHUNK_KEY][position < 0 ? "-" : "+"].PushBack(
				rapidjson::Value(rapidjson::kArrayType).SetObject().Move(), this->mData.GetAllocator());
		}
	};

	const auto save = [this, &pad](Chunk* chunk) {
		pad(chunk->getPosition());

		this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())].SetObject();

		chunk->save(this->mScene,
			    this->mData[CHUNK_KEY][chunk->getPosition() < 0 ? "-" : "+"][SDL_abs(chunk->getPosition())],
			    this->mData.GetAllocator());

		delete chunk;
	};

	if (currentChunk == mLeft->getPosition()) {
		save(mRight);

		mRight = mCenter;
		mCenter = mLeft;

		pad(currentChunk - 1);
		const auto& chunkData = mData[CHUNK_KEY][(currentChunk - 1) < 0 ? "-" : "+"][SDL_abs(currentChunk - 1)];
		if (chunkData.IsNull() || !chunkData.HasMember("blocks")) {
			SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM, "\033[31mGenerating new chunk for chunk %d\033[0m",
				    currentChunk - 1);

			mLeft = new Chunk(mScene, mNoise.get(), currentChunk - 1);
		} else {
			mLeft = new Chunk(chunkData, mScene);
		}
	} else if (currentChunk == mRight->getPosition()) {
		save(mLeft);

		mLeft = mCenter;
		mCenter = mRight;

		pad(currentChunk + 1);
		const auto& chunkData = mData[CHUNK_KEY][(currentChunk + 1) < 0 ? "-" : "+"][SDL_abs(currentChunk + 1)];
		if (chunkData.IsNull() || !chunkData.HasMember("blocks")) {
			SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM, "\033[31mGenerating new chunk for chunk %d\033[0m",
				    currentChunk + 1);

			mRight = new Chunk(mScene, mNoise.get(), currentChunk + 1);
		} else {
			mRight = new Chunk(chunkData, mScene);
		}
	} else {
		SDL_Log("\033[33mOut of boundary for chunk %d, loaded chunks: %" PRIi64 " %" PRIi64 " %" PRIi64
			"\033[0m",
			currentChunk, mLeft->getPosition(), mCenter->getPosition(), mRight->getPosition());

		save(mLeft);
		save(mCenter);
		save(mRight);

		mLeft = new Chunk(mScene, mNoise.get(), currentChunk - 1);
		mCenter = new Chunk(mScene, mNoise.get(), currentChunk);
		mRight = new Chunk(mScene, mNoise.get(), currentChunk + 1);
	}
}

void Level::createCommon() {
	const auto player = mGame->getPlayerID();
	auto* const playerTexture = mGame->getSystemManager()->getTexture("steve.png", true);
	mScene->emplace<Components::animated_texture>(player, playerTexture, Eigen::Vector2i(8, 7), 0);
	mScene->emplace<Components::collision>(
		player, Eigen::Vector2f(4.0f * 7.0f, 0.0f),
		Eigen::Vector2f(Components::block::BLOCK_SIZE, Components::block::BLOCK_SIZE));
	mScene->emplace<Components::misc>(player, Components::misc::JUMP | Components::misc::PLAYER);
	mScene->emplace<Components::input>(player, [](class Scene* scene, const EntityID entity, const float) {
		Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).mVelocity;

		if (scene->getSignal(SDL_SCANCODE_D) && vel.x() < 340) {
			vel.x() += 100;
		}

		if (scene->getSignal(SDL_SCANCODE_A) && vel.x() > -340) {
			vel.x() -= 100;
		}

		// Open inv
		if (scene->getSignal(SDL_SCANCODE_E)) {
			Game::getInstance()->getSystemManager()->getUISystem()->addScreen(
				scene->get<Components::inventory>(entity).mInventory);
		}
	});

	mTextID = mScene->newEntity();
	mScene->emplace<Components::text>(mTextID, "AD0");
	mScene->emplace<Components::position>(mTextID, Eigen::Vector2f(std::numeric_limits<float>::quiet_NaN(),
								       std::numeric_limits<float>::infinity()));
}

std::int64_t Level::getPosition() { return mCenter->getPosition(); }

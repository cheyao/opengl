#pragma once

#include "managers/entityManager.hpp"
#include "third_party/json.hpp"

#include <cstdint>
#include <string>

// Something important
class Level {
      public:
	explicit Level(class Game* game, const std::string& name = "Level");
	Level(Level&&) = delete;
	Level(const Level&) = delete;
	Level& operator=(Level&&) = delete;
	Level& operator=(const Level&) = delete;
	~Level();

	void create();
	void load(const nlohmann::json& data);
	nlohmann::json save();

	std::string getName() const { return mName; }
	class Scene* getScene() const { return mScene; };

	// The update method maily checks if new chunks needs to be loaded
	void update(float delta);

      private:
	inline constexpr const static char* const CHUNK_KEY = "chunks";
	inline constexpr const static char* const PLAYER_KEY = "player";
	inline constexpr const static uint64_t ROLL_TIME = 5000;

	void createCommon();

	const std::string mName;
	EntityID mTextID;
	uint64_t mLastTime;

	nlohmann::json mData;

	// We only need three chunks max loaded at once
	class Chunk* mLeft;
	class Chunk* mCenter;
	class Chunk* mRight;

	class Game* mGame;
	class Scene* mScene;
};

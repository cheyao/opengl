#pragma once

#include "third_party/json.hpp"

#include <string>

// Something important
class Level {
      public:
	explicit Level(class Game* game, const std::string& name = "Level");
	Level(Level&&) = delete;
	Level(const Level&) = delete;
	Level& operator=(Level&&) = delete;
	Level& operator=(const Level&) = delete;
	~Level() = default;

	void create();
	void load(const nlohmann::json& data);
	nlohmann::json& save();

	std::string getName() const { return mName; }
	class Scene* getScene() const { return mScene; };

	// The update method maily checks if new chunks needs to be loaded
	void update();

      private:
	inline constexpr const static char* const CHUNK_KEY = "chunks";
	inline constexpr const static char* const PLAYER_KEY = "player";

	const std::string mName;

	nlohmann::json mData;

	class Game* mGame;
	class Scene* mScene;

	// We only need three chunks max loaded at once
	class Chunk* mLeft;
	class Chunk* mCenter;
	class Chunk* mRight;
};

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
	virtual ~Level() = default;

	virtual void create();
	virtual void load(const nlohmann::json data);
	virtual nlohmann::json save();

	std::string getName() const { return mName; }
	class Scene* getScene() const { return mScene; };

      private:
	const std::string mName;

	class Game* mGame;
	class Scene* mScene;

	std::vector<class Chunk*> mChunks;
};

#pragma once

#include "third_party/json.hpp"

#include <string>

// Something important
class Level {
      public:
	explicit Level(const std::string name);
	Level(Level&&) = default;
	Level(const Level&) = default;
	Level& operator=(Level&&) = default;
	Level& operator=(const Level&) = default;
	~Level() = default;

	virtual void create();
	virtual void load(const nlohmann::json data, class Game* game);
	virtual nlohmann::json save(class Game* game);

	std::string getName() const { return mName; }

      private:
	std::string mName;
};

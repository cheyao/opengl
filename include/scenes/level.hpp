#pragma once

#include "third_party/json.hpp"

// Something important
class Level {
      public:
	Level() = default;
	Level(Level&&) = default;
	Level(const Level&) = default;
	Level& operator=(Level&&) = default;
	Level& operator=(const Level&) = default;
	~Level() = default;

	virtual void create();
	virtual void load(const nlohmann::json data);
	virtual nlohmann::json save();

      private:
};

#pragma once

#include "actors/actor.hpp"

class View : public Actor {
      public:
	View(class Game* game);
	View(View&&) = delete;
	View(const View&) = delete;
	View& operator=(View&&) = delete;
	View& operator=(const View&) = delete;
	~View() override = default;

      private:
};

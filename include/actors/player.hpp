#pragma once

#include "actors/actor.hpp"

class Player : public Actor {
      public:
	explicit Player(class Game* game);
	Player(Player&&) = delete;
	Player(const Player&) = delete;
	Player& operator=(Player&&) = delete;
	Player& operator=(const Player&) = delete;
	~Player() override = default;

      private:
};

#pragma once

#include "actors/actor.hpp"

class Player : public Actor {
public:
	Player();
	Player(Player &&) = delete;
	Player(const Player &) = delete;
	Player &operator=(Player &&) = delete;
	Player &operator=(const Player &) = delete;
	~Player();

private:
	
};


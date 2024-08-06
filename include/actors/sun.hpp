#pragma once

#include "actors/actor.hpp"

class Sun : public Actor {
  public:
	explicit Sun(class Game* game);
	Sun(Sun&&) = delete;
	Sun(const Sun&) = delete;
	Sun& operator=(Sun&&) = delete;
	Sun& operator=(const Sun&) = delete;
	~Sun() override = default;

	void updateActor(float delta) override;

  private:
};

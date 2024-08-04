#pragma once

#include "actors/actor.hpp"

class Cube : public Actor {
  public:
	explicit Cube(class Game* owner);
	Cube(Cube&&) = delete;
	Cube(const Cube&) = delete;
	Cube& operator=(Cube&&) = delete;
	Cube& operator=(const Cube&) = delete;
	~Cube() override = default;

  private:
};

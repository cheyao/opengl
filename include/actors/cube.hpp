#pragma once

#include "actors/actor.hpp"
#include "opengl/vertexArray.hpp"

class Cube : public Actor {
  public:
	explicit Cube(class Game* game);
	Cube(Cube&&) = delete;
	Cube(const Cube&) = delete;
	Cube& operator=(Cube&&) = delete;
	Cube& operator=(const Cube&) = delete;
	~Cube() override;

  private:
	class VertexArray* mVertex;
};

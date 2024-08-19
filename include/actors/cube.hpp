#pragma once

#include "actors/actor.hpp"
#include "components/modelComponent.hpp"

class Cube : public Actor {
      public:
	explicit Cube(class Game* owner);
	Cube(Cube&&) = delete;
	Cube(const Cube&) = delete;
	Cube& operator=(Cube&&) = delete;
	Cube& operator=(const Cube&) = delete;
	~Cube() override = default;

	void updateActor([[maybe_unused]] float delta) override;

      private:
	ModelComponent* mMeteorites;
};

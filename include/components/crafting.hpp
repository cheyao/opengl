#pragma once

#include "components.hpp"
#include "screens/screen.hpp"
#include <cstdint>

class Crafting : public Screen {
      public:
	explicit Crafting(class Game* game, std::uint64_t row, std::uint64_t col);
	Crafting(Crafting&&) = delete;
	Crafting(const Crafting&) = delete;
	Crafting& operator=(Crafting&&) = delete;
	Crafting& operator=(const Crafting&) = delete;
	~Crafting() override = default;

	bool update(class Scene* scene, float delta) override;
	void draw(class Scene* scene) override;

      private:
	class Game* mGame;

	std::vector<Components::Item> mGrid;
};

#pragma once

#include "screens/screen.hpp"

// Responsible for the bottom menu bar
class HUD : public Screen {
      public:
	explicit HUD(class Game* game);
	HUD(HUD&&) = delete;
	HUD(const HUD&) = delete;
	HUD& operator=(HUD&&) = delete;
	HUD& operator=(const HUD&) = delete;
	~HUD() override = default;

      private:
};

#pragma once

#include "ui/UIScreen.hpp"

class Controls : public UIScreen {
      public:
	explicit Controls(class Game* game);
	Controls(Controls&&) = delete;
	Controls(const Controls&) = delete;
	Controls& operator=(Controls&&) = delete;
	Controls& operator=(const Controls&) = delete;
	~Controls() override;

      private:
};


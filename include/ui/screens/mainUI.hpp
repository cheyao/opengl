#pragma once

#include "ui/UIScreen.hpp"

class MainUI : public UIScreen {
      public:
	explicit MainUI(class Game* game);
	MainUI(MainUI&&) = delete;
	MainUI(const MainUI&) = delete;
	MainUI& operator=(MainUI&&) = delete;
	MainUI& operator=(const MainUI&) = delete;
	~MainUI() = default;

      private:
};

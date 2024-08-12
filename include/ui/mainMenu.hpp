#pragma once

#include "ui/UIScreen.hpp"

class MainMenu : public UIScreen {
      public:
	explicit MainMenu(class Game* game);
	MainMenu(MainMenu&&) = delete;
	MainMenu(const MainMenu&) = delete;
	MainMenu& operator=(MainMenu&&) = delete;
	MainMenu& operator=(const MainMenu&) = delete;
	~MainMenu() = default;

      private:
};

#pragma once

#include "ui/UIScreen.hpp"

class ControlUI : public UIScreen {
      public:
	explicit ControlUI(class Game* game);
	ControlUI(ControlUI&&) = delete;
	ControlUI(const ControlUI&) = delete;
	ControlUI& operator=(ControlUI&&) = delete;
	ControlUI& operator=(const ControlUI&) = delete;
	~ControlUI() override;

      private:
};

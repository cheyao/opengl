#pragma once

#include "ui/UIScreen.hpp"

class DivUI : public UIScreen {
      public:
	explicit DivUI(class Game* owner, class UIScreen* parent);
	DivUI(DivUI&&) = delete;
	DivUI(const DivUI&) = delete;
	DivUI& operator=(DivUI&&) = delete;
	DivUI& operator=(const DivUI&) = delete;
	~DivUI() override;

	void updateScreen(const float delta) override;

      private:
	class UIScreen* mParent;
};

#pragma once

class UIScreen {
      public:
	UIScreen();
	UIScreen(UIScreen&&) = delete;
	UIScreen(const UIScreen&) = delete;
	UIScreen& operator=(UIScreen&&) = delete;
	UIScreen& operator=(const UIScreen&) = delete;
	~UIScreen();

      private:
};

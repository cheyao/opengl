#pragma once

class Screen {
      public:
	Screen() = default;
	Screen(Screen&&) = delete;
	Screen(const Screen&) = delete;
	Screen& operator=(Screen&&) = delete;
	Screen& operator=(const Screen&) = delete;
	~Screen() = default;

	void update();
	void render();

      private:
};

#pragma once

#include <SDL3/SDL.h>

#include <string>

class Game {
  public:
	Game();
	~Game();

	int init();
	int iterate();
	int event(const SDL_Event &event);

	// No copy
	Game(const Game &) = delete;
	Game &operator=(const Game &x) = delete;

  private:
	void input();
	void update();
	void gui();
	void draw();

	SDL_Window *mWindow;
	SDL_GLContext mContext;

	Uint64 mTicks;

	std::string mBasePath;

	unsigned int mVAO;
	unsigned int mShaderProgram;
};

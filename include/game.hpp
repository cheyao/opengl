#pragma once

#include <SDL3/SDL.h>

#include <string>

#define fullPath(path) (mBasePath + "assets/" + path)

class Game {
  public:
	Game();
	Game(Game&&) = delete;
	Game(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	Game& operator=(const Game&) = delete;
	~Game();

	int iterate();
	int event(const SDL_Event& event);

  private:
	void input();
	void update();
	void gui();
	void draw();
	void setup();

	SDL_Window* mWindow;
	SDL_GLContext mContext;

	class Shader* mShader;
	class VertexArray* mVertex;

	Uint64 mTicks;

	std::string mBasePath;

	class Texture* mBox;
	class Texture* mFace;

	float mixer;

	// TODO: Seperate InputHandler class
	// TODO: Seperate draw class
	// TODO: Seperate Texture manager class
};

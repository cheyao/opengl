#pragma once

#include <SDL3/SDL.h>

class GLManager {
public:
	GLManager();
	GLManager(GLManager &&) = delete;
	GLManager(const GLManager &) = delete;
	GLManager &operator=(GLManager &&) = delete;
	GLManager &operator=(const GLManager &) = delete;
	~GLManager();

	static void printInfo();
	void bindContext(SDL_Window* window);

	SDL_GLContext getContext() { return mContext; }

private:
	SDL_GLContext mContext;
};


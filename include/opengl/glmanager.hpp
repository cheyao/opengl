#pragma once

#include <SDL3/SDL.h>

class GLManager {
public:
	explicit GLManager(SDL_Window* window);
	GLManager(GLManager &&) = delete;
	GLManager(const GLManager &) = delete;
	GLManager &operator=(GLManager &&) = delete;
	GLManager &operator=(const GLManager &) = delete;
	~GLManager();

	void printInfo() const;

	SDL_GLContext getContext() { return mContext; }

private:
	SDL_GLContext mContext;
};


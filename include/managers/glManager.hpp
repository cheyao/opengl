#pragma once

#include <SDL3/SDL.h>

#ifdef DEBUG
#define GL_CHECK(stmt)                                                                             \
	do {                                                                                           \
		stmt;                                                                                      \
		checkGLError(#stmt, __FILE__, __LINE__);                                                   \
	} while (0)
#else
#define GL_CHECK(stmt)
#endif

void checkGLError(const char* stmt, const char* fname, int line);

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


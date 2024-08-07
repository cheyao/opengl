#pragma once

#include <cmath>
#include <SDL3/SDL.h>

#define ERROR_BOX(msg) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", msg, nullptr)

const float PI = 3.14159265358979323846;

inline float toRadians(float degrees) { return degrees * (PI / 180.f); }

inline float toDegrees(float radians) { return radians * (180.f / PI); }

inline bool nearZero(float number, float epsilon = 0.001f) {
	if (fabs(number) <= epsilon) {
		return true;
	} else {
		return false;
	}
}

#if defined(__MINGW32__) || defined(__WIN32__) || defined(__CYGWIN32__) || defined(__WINDOWS__)
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

#define CHECKERROR() \
	{ \
	GLenum err; \
	while ((err = glGetError()) != GL_NO_ERROR) { \
		switch (err) { \
			case GL_INVALID_ENUM: \
				SDL_Log("GLError: Invalid enum"); \
				break; \
			case GL_INVALID_VALUE: \
				SDL_Log("GLError: Invalid value"); \
				break; \
			case GL_INVALID_OPERATION: \
				SDL_Log("GLError: Invalid operation"); \
				break; \
			case GL_INVALID_FRAMEBUFFER_OPERATION: \
				SDL_Log("GLError: Invalid framebuffer op"); \
				break; \
			case GL_OUT_OF_MEMORY: \
				SDL_Log("GLError: Out of memory"); \
				break; \
		} \
		throw 1; \
	} \
	}


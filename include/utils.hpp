#pragma once

#include <SDL3/SDL.h>

#define ERROR_BOX(msg) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", msg, nullptr)

constexpr const static float PI = 3.14159265358979323846;

inline constexpr float toRadians(const float degrees) { return degrees * (PI / 180.f); }

inline constexpr float toDegrees(const float radians) { return radians * (180.f / PI); }

inline constexpr bool nearZero(const float number, const float epsilon = 0.001f) {
	if (SDL_fabs(number) <= epsilon) {
		return true;
	} else {
		return false;
	}
}

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__)
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

#define CHECKERROR()                                                                                                   \
	{                                                                                                              \
		GLenum err;                                                                                            \
		while ((err = glGetError()) != GL_NO_ERROR) {                                                          \
			switch (err) {                                                                                 \
				case GL_INVALID_ENUM:                                                                  \
					SDL_Log("GLError: Invalid enum");                                              \
					break;                                                                         \
				case GL_INVALID_VALUE:                                                                 \
					SDL_Log("GLError: Invalid value");                                             \
					break;                                                                         \
				case GL_INVALID_OPERATION:                                                             \
					SDL_Log("GLError: Invalid operation");                                         \
					break;                                                                         \
				case GL_INVALID_FRAMEBUFFER_OPERATION:                                                 \
					SDL_Log("GLError: Invalid framebuffer op");                                    \
					break;                                                                         \
				case GL_OUT_OF_MEMORY:                                                                 \
					SDL_Log("GLError: Out of memory");                                             \
					break;                                                                         \
			}                                                                                              \
			SDL_Log("Check thrown");                                                                       \
			throw 1;                                                                                       \
		}                                                                                                      \
	}

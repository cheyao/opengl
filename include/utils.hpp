#pragma once

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#pragma warning(disable : 4996)
#endif

#include <SDL3/SDL.h>
#include <memory>
#include <stdexcept>
#include <string>

template <typename... Args> std::string string_format(const std::string& format, Args... args) {
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
	if (size_s <= 0) {
		throw std::runtime_error("Error during formatting.");
	}
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args...);
	return std::string(buf.get(), buf.get() + size - 1);
}

#define ERROR_BOX(msg)                                                                                                 \
	{                                                                                                              \
		if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", msg, nullptr)) {                          \
			SDL_Log("Failed to show message box: %s", SDL_GetError());                                     \
		}                                                                                                      \
	}

constexpr const static float PI = 3.14159265358979323846;
constexpr const static float EULER = 2.71828182845904523536;

inline constexpr float toRadians(const float degrees) { return degrees * (PI / 180.f); }

inline constexpr float toDegrees(const float radians) { return radians * (180.f / PI); }

inline constexpr bool nearZero(const float number, const float epsilon = 0.001f) {
	if (SDL_fabs(number) <= epsilon) {
		return true;
	} else {
		return false;
	}
}

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
			throw std::runtime_error("OpenGL error thrown! (Check logs)");                                 \
		}                                                                                                      \
	}

void* loadFile(const char* file, size_t* datasize);

#pragma once

#include <third_party/glad/glad.h>

#include <SDL3/SDL.h>

#define ERROR_BOX(msg) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", msg, nullptr)

const float PI = 3.14159265358979323846;

inline float toRadians(float degrees) { return degrees * (PI / 180.f); }

inline float toDegrees(float radians) { return radians * (180.f / PI); }


#include "utils.hpp"

#include <SDL3/SDL.h>

void* loadFile(const char* file, size_t* datasize) { return SDL_LoadFile(file, datasize); }

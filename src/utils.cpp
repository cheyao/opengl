#include "utils.hpp"

#include <SDL3/SDL.h>
#include <vector>

namespace utils {
	class sparse_set_interface;
}

void* loadFile(const char* file, size_t* datasize) { return SDL_LoadFile(file, datasize); }
std::vector<int*> mViewCache;
std::vector<class utils::sparse_set_interface*> mPools;

#include "utils.hpp"

#include <SDL3/SDL.h>
#include <string>
#include <vector>

namespace utils {
class sparse_set_interface;
}

void* loadFile(const char* file, size_t* datasize) { return SDL_LoadFile(file, datasize); }
std::vector<int*> mViewCache;

std::string getBasePath_impl() {
	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		return std::string(basepath);
	} else {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\033[31mFailed to get base path: %s\033[0m", SDL_GetError());

		return "";
	}
}

extern std::string getBasePath() {
	static const auto path = getBasePath_impl();
	return path;
}

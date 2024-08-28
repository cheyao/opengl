#include "managers/storageManager.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <format>
#include <stdexcept>

StorageManager::StorageManager() {
	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to open user storage: %s\n", SDL_GetError());
		ERROR_BOX("Failed to open storage");

		throw std::runtime_error("StorageManager.cpp: Failed to open storage");
	}

	size_t times = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);

		++times;
		if (times >= 100) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to open user storage: %s\n", SDL_GetError());
			ERROR_BOX("Failed to open storage");

			throw std::runtime_error("StorageManager.cpp: Failed to open storage");
		}
	}

	int c;
	char** paths = SDL_GlobStorageDirectory(storage, "", nullptr, SDL_GLOB_CASEINSENSITIVE, &c);
	if (paths == nullptr) {

		SDL_Log("%s", SDL_GetError());

		throw std::runtime_error("StorageManager.cpp: Failed to open storage");
	}

	for (int i = 0; i < c; i++) {
		SDL_Log("%s", paths[i]);
	}

	SDL_free(paths);

	SDL_CloseStorage(storage);
}

StorageManager::~StorageManager() {
	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to open user storage: %s\n", SDL_GetError());
		ERROR_BOX("Failed to open storage: Not saving");

		return;
	}

	size_t i = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);
		++i;
		if (i >= 100) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to open user storage: %s\n", SDL_GetError());
			ERROR_BOX("Failed to open storage: Not saving");

			return;
		}
	}

	SDL_Log("%sGib space remaining in storage device:",
		std::format("{}", SDL_GetStorageSpaceRemaining(storage) / 1024 / 1024 / 1024).data());

	std::string s = "Hello world!";

	if (!SDL_WriteStorageFile(storage, "hello.txt", s.data(), s.size())) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to write save file: %s\n", SDL_GetError());
		ERROR_BOX("Failed to write save file");

		return;
	}

	SDL_CloseStorage(storage);
}

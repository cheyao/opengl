#include "managers/storageManager.hpp"

#include "third_party/json.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstdint>
#include <stdexcept>

StorageManager::StorageManager(Game* game) : mGame(game) {
	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m", SDL_GetError());
		ERROR_BOX("Failed to open storage");

		throw std::runtime_error("StorageManager.cpp: Failed to open storage");
	}

	size_t times = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);

		++times;
		if (times >= 100) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m",
				     SDL_GetError());
			ERROR_BOX("Failed to open storage");

			throw std::runtime_error("StorageManager.cpp: Failed to open storage");
		}
	}

	restoreState(storage);

	SDL_CloseStorage(storage);
}

StorageManager::~StorageManager() {
	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m", SDL_GetError());
		ERROR_BOX("Failed to open storage: Not saving");

		return;
	}

	size_t i = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);
		++i;
		if (i >= 100) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m",
				     SDL_GetError());
			ERROR_BOX("Failed to open storage: Not saving");

			return;
		}
	}

	saveState(storage);

	SDL_CloseStorage(storage);
}

void StorageManager::restoreState(SDL_Storage* storage) { (void)storage; }

void StorageManager::saveState(SDL_Storage* storage) {
	nlohmann::json worlds; // TODO: Levels
	bool oldWorlds = false;

	// If there is already level data
	if (SDL_GetStoragePathInfo(storage, "worlds.json", nullptr)) {
		oldWorlds = true;

		SDL_PathInfo info;
		SDL_GetStoragePathInfo(storage, "worlds.json", &info);

		std::uint8_t* buffer = new std::uint8_t[info.size + 1];
		SDL_ReadStorageFile(storage, "worlds.json", buffer, info.size);
		buffer[info.size] = 0; // Ensure null terminasion

		try {
			worlds = nlohmann::json::parse(buffer);
		} catch (...) {
			// Ignore, write new one
			SDL_Log("\x1B[31mError! Failed to parse worlds.json! (Unknown exception)\033[0m");
		}

		delete[] buffer;
	}

	// FIXME: Non-hard writted world name
	constexpr const static char* worldName = "world";

	// FIXME: Version
	worlds["version"] = 100;
	// TODO: Save some world info
	worlds[worldName] = true;

	if (oldWorlds && !SDL_RenameStoragePath(storage, "worlds.json", "worlds.json.old")) {
		SDL_Log("\x1B[31mFailed to rename world.json to worlds.json.old, continuing... %s\033[0m",
			SDL_GetError());
	}

	SDL_WriteStorageFile(storage, "worlds.json", worlds.dump().data(), worlds.dump().size());
}

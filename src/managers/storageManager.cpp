#include "managers/storageManager.hpp"

#include "third_party/json.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstdint>
#include <stdexcept>
#include <string>

StorageManager::StorageManager(Game* game) : mGame(game) {}

void StorageManager::restore() {
	SDL_Log("Restoring state");

	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m",
				SDL_GetError());
		ERROR_BOX("Failed to open storage");

		throw std::runtime_error("StorageManager.cpp: Failed to open storage");
	}

	size_t times = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);

		++times;
		if (times >= 100) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m",
					SDL_GetError());
			ERROR_BOX("Failed to open storage");

			throw std::runtime_error("StorageManager.cpp: Failed to open storage");
		}
	}

	try {
		restoreState(storage);
	} catch (const nlohmann::json::parse_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to parse json: id %d %s at %zu\033[0m",
				error.id, error.what(), error.byte);
		ERROR_BOX("Failed to parse json");

		throw std::runtime_error("StorageManager.cpp: Failed to parse json");
	}

	SDL_CloseStorage(storage);

	SDL_Log("\x1B[32Successfully loaded state\033[0m");
}

StorageManager::~StorageManager() {
	SDL_Log("Saving state");

	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m",
				SDL_GetError());
		ERROR_BOX("Failed to open storage: Not saving");

		return;
	}

	size_t i = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);
		++i;
		if (i >= 100) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to open user storage: %s\033[0m",
					SDL_GetError());
			ERROR_BOX("Failed to open storage: Not saving");

			return;
		}
	}

	try {
		saveState(storage);
	} catch (const nlohmann::json::parse_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to parse json: id %d %s at %zu\033[0m",
				error.id, error.what(), error.byte);
		ERROR_BOX("Failed to parse json");

		return;
	}

	SDL_CloseStorage(storage);

	SDL_Log("\x1B[32Successfully saved state\033[0m");
}

// TODO: Binary json https://json.nlohmann.me/features/binary_formats/
void StorageManager::restoreState(SDL_Storage* storage) {
	if (!SDL_GetStoragePathInfo(storage, "worlds.json", nullptr)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mNo save index file found! Loading new state\033[0m");

		throw std::runtime_error("No save index file");
	}

	nlohmann::json worlds;

	SDL_PathInfo info;
	SDL_GetStoragePathInfo(storage, "worlds.json", &info);

	std::uint8_t* buffer = new std::uint8_t[info.size + 1];
	SDL_ReadStorageFile(storage, "worlds.json", buffer, info.size);
	buffer[info.size] = 0; // Ensure null terminasion

	try {
		worlds = nlohmann::json::parse(buffer);
	} catch (const nlohmann::json::parse_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to parse json: id %d %s at %zu\033[0m",
				error.id, error.what(), error.byte);

		throw error;
	}

	SDL_assert(worlds["version"] == 100);
	for (const auto& world : worlds.get<std::vector<std::string>>()) {
		SDL_Log("Found world %s", world.data());
	}

	delete[] buffer;
}

void StorageManager::loadWorld(struct SDL_Storage* storage, const std::string& world) {
	(void)storage;
	(void)world;
}

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
		} catch (const nlohmann::json::parse_error& error) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to parse json: id %d %s at %zu\033[0m",
					error.id, error.what(), error.byte);
		}

		delete[] buffer;
	}

	// FIXME: Non-hard writted world name
	constexpr const static char* worldName = "world";

	// FIXME: Version
	worlds["version"] = 100;
	// TODO: Save some world info
	worlds["worlds"] = {worldName};

	if (oldWorlds && !SDL_RenameStoragePath(storage, "worlds.json", "worlds.json.old")) {
		SDL_Log("\x1B[31mFailed to rename world.json to worlds.json.old %s, ignoring...\033[0m",
			SDL_GetError());
	}

	SDL_WriteStorageFile(storage, "worlds.json", worlds.dump().data(), worlds.dump().size());
}

// void StorageManager::saveWorld(struct SDL_Storage* storage) {}

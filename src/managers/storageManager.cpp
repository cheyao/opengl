#include "managers/storageManager.hpp"

#include "game.hpp"
#include "scenes/level.hpp"
#include "third_party/json.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/*
 * A World shall consist of 3 fields:
 * 1. "version"
 * 2. "name"
 * 3. "data"
 */

StorageManager::StorageManager(Game* const game) : mGame(game) {}

StorageManager::~StorageManager() {
	SDL_Log("Saving state");

	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to open user storage: %s\033[0m",
				SDL_GetError());
		ERROR_BOX("Failed to open storage: Not saving");

		return;
	}

	std::size_t i = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);
		++i;
		if (i >= 100) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to open user storage: %s\033[0m",
					SDL_GetError());
			ERROR_BOX("Failed to open storage: Abandoning saving operation");

			return;
		}
	}

	try {
		saveState(storage);
	} catch (const nlohmann::json::parse_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to parse json: id %d %s at %zu\033[0m",
				error.id, error.what(), error.byte);
		ERROR_BOX("Failed to parse json");

		return;
	}

	SDL_CloseStorage(storage);

	SDL_Log("\033[32mSuccessfully saved state\033[0m");
}

void StorageManager::restore() {
	SDL_Log("Restoring state");

	SDL_Storage* storage = SDL_OpenUserStorage("cyao", "opengl", 0);
	if (storage == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to open user storage: %s\033[0m",
				SDL_GetError());
		ERROR_BOX("Failed to open storage");

		throw std::runtime_error("StorageManager.cpp: Failed to open storage");
	}

	std::size_t times = 0;
	while (!SDL_StorageReady(storage)) {
		SDL_Delay(10);

		++times;
		if (times >= 100) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to open user storage: %s\033[0m",
					SDL_GetError());
			ERROR_BOX("Failed to open storage");

			throw std::runtime_error("StorageManager.cpp: Failed to open storage");
		}
	}

	try {
		restoreState(storage);
	} catch (const nlohmann::json::parse_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to parse json: id %d %s at %zu\033[0m",
				error.id, error.what(), error.byte);
		ERROR_BOX("Failed to parse level file");

		SDL_CloseStorage(storage);

		throw std::runtime_error("StorageManager.cpp: Failed to parse json");
	}

	SDL_CloseStorage(storage);

	SDL_Log("\033[32mSuccessfully loaded state\033[0m");
}

// TODO: Binary json https://json.nlohmann.me/features/binary_formats/
void StorageManager::restoreState(SDL_Storage* storage) {
	if (!SDL_GetStoragePathInfo(storage, "worlds.json", nullptr)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mNo save index file found! Loading new state\033[0m");

		throw std::runtime_error("No save index file");
	}

	SDL_PathInfo info;
	SDL_GetStoragePathInfo(storage, "worlds.json", &info);

	std::uint8_t* buffer = new std::uint8_t[info.size + 1];
	SDL_ReadStorageFile(storage, "worlds.json", buffer, info.size);
	buffer[info.size] = 0; // Ensure null terminasion

	nlohmann::json worlds;
	try {
		worlds = nlohmann::json::parse(buffer);

		delete[] buffer;
	} catch (const nlohmann::json::exception& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to parse json: id %d %s\033[0m", error.id,
				error.what());

		delete[] buffer;

		throw error;
	}

	if (worlds["version"] != LATEST_WORLD_VERSION) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
				"\033[31mERROR! The save file version is not for this game engine version\033[0m");
		ERROR_BOX("ERROR! The save file version is not for this game engine version");

		throw std::runtime_error("StorageManager.cpp: Failed to read wrong version save file");
	}

	if (worlds["worlds"].empty()) {
		SDL_Log("\033[31mWorlds empty! Returning\033[0m");

		return;
	}

	for (const auto& world : worlds["worlds"]) {
		SDL_Log("Found world %s", world.get<std::string>().data());
		SDL_assert(world.get<std::string>() != "worlds");
	}

	loadWorld(storage, worlds["worlds"][0].get<std::string>());
}

void StorageManager::loadWorld(struct SDL_Storage* storage, const std::string& world) {
	if (!SDL_GetStoragePathInfo(storage, (world + ".json").data(), nullptr)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mNo save file %s found! Loading new state\033[0m",
				world.data());

		throw std::runtime_error("No save index file");
	}

	SDL_PathInfo info;
	SDL_GetStoragePathInfo(storage, (world + ".json").data(), &info);

	std::uint8_t* buffer = new std::uint8_t[info.size + 1];
	SDL_ReadStorageFile(storage, (world + ".json").data(), buffer, info.size);
	buffer[info.size] = 0; // Ensure null terminasion

	nlohmann::json level;
	try {
		level = nlohmann::json::parse(buffer);

		delete[] buffer;
	} catch (const nlohmann::json::parse_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to parse json %s: id %d %s at %zu\033[0m",
				world.data(), error.id, error.what(), error.byte);

		delete[] buffer;

		throw error;
	}

	if (level["version"] != LATEST_LEVEL_VERSION) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
				"\033[31mERROR! The save file version is not for this game engine version\033[0m");
		ERROR_BOX("ERROR! The save file version is not for this game engine version");

		throw std::runtime_error("StorageManager.cpp: Failed to read wrong version save file");
	}

	SDL_assert(level["name"] == "Level"); // TODO: More option

	mGame->mCurrentLevel = new Level(mGame);
	mGame->mCurrentLevel->load(level["data"]);
}

void StorageManager::saveState(SDL_Storage* storage) {
	nlohmann::json worlds;
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
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to parse json: id %d %s at %zu\033[0m",
					error.id, error.what(), error.byte);
		}

		delete[] buffer;
	}

	// FIXME: Non-hard writted world name, ask for user input
	constexpr const static char* worldName = "world";

	// TODO: Save some more world info
	// TODO: Self-recovery incase of corrupted world save
	worlds["version"] = LATEST_WORLD_VERSION;

	if (worlds.contains("worlds")) {
		const auto& worldList = worlds["worlds"].get<std::vector<std::string_view>>();

		if (std::find(worldList.begin(), worldList.end(), worldName) == worldList.end()) {
			worlds["worlds"] += worldName;
		}
	} else {
		worlds["worlds"] = {worldName};
	}

	if (oldWorlds && !SDL_RenameStoragePath(storage, "worlds.json", "worlds.json.old")) {
		SDL_Log("\033[31mFailed to rename world.json to worlds.json.old %s, ignoring...\033[0m",
			SDL_GetError());
	}

	SDL_WriteStorageFile(storage, "worlds.json", worlds.dump().data(), worlds.dump().size());

	saveWorld(storage, worldName);
}

void StorageManager::saveWorld(struct SDL_Storage* storage, const std::string& world) {
	if (SDL_GetStoragePathInfo(storage, (world + ".json").data(), nullptr)) {
		if (!SDL_RenameStoragePath(storage, (world + ".json").data(), (world + ".json.old").data())) {
			SDL_Log("\033[31mFailed to rename world.json to world.json.old %s, ignoring...\033[0m",
				SDL_GetError());
		}
	}

	nlohmann::json level;

	level["version"] = LATEST_LEVEL_VERSION;
	level["name"] = mGame->mCurrentLevel->getName(); // TODO: More option
	level["data"] = mGame->mCurrentLevel->save();

	SDL_WriteStorageFile(storage, (world + ".json").data(), level.dump().data(), level.dump().size());
}

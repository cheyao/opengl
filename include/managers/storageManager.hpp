#pragma once

#include <string>

// When object is destoryed, save state, when crated restor state
class StorageManager {
      public:
	explicit StorageManager();
	StorageManager(StorageManager&&) = delete;
	StorageManager(const StorageManager&) = delete;
	StorageManager& operator=(StorageManager&&) = delete;
	StorageManager& operator=(const StorageManager&) = delete;
	~StorageManager();

	[[nodiscard]] bool restore();
	void save();

      private:
	[[nodiscard]] bool restoreState(struct SDL_Storage* storage);
	[[nodiscard]] bool loadWorld(struct SDL_Storage* storage, const std::string& world);
	void saveState(struct SDL_Storage* storage);
	void saveWorld(struct SDL_Storage* storage, const std::string& world);

	constexpr const static inline unsigned long long LATEST_LEVEL_VERSION = 100;
	constexpr const static inline unsigned long long LATEST_WORLD_VERSION = 100;

	class Game* const mGame;
};

#pragma once

#include <string>

// When object is destoryed, save state, when crated restor state
class StorageManager {
      public:
	explicit StorageManager(class Game* game);
	StorageManager(StorageManager&&) = delete;
	StorageManager(const StorageManager&) = delete;
	StorageManager& operator=(StorageManager&&) = delete;
	StorageManager& operator=(const StorageManager&) = delete;
	~StorageManager();

	void restore();

      private:
	void restoreState(struct SDL_Storage* storage);
	void loadWorld(struct SDL_Storage* storage, const std::string& world);
	void saveState(struct SDL_Storage* storage);
	void saveWorld(struct SDL_Storage* storage, const std::string& world);

	class Game* mGame;
};

#pragma once

#include <SDL3/SDL.h>

// When object is destoryed, save state, when crated restor state
class StorageManager {
      public:
	explicit StorageManager(class Game* game);
	StorageManager(StorageManager&&) = delete;
	StorageManager(const StorageManager&) = delete;
	StorageManager& operator=(StorageManager&&) = delete;
	StorageManager& operator=(const StorageManager&) = delete;
	~StorageManager();

      private:
	void restoreState(struct SDL_Storage* storage);
	void saveState(struct SDL_Storage* storage);

	class Game* mGame;
};

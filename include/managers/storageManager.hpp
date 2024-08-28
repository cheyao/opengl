#pragma once

#include <SDL3/SDL.h>

// When object is destoryed, save state, when crated restor state
class StorageManager {
      public:
	StorageManager();
	StorageManager(StorageManager&&) = delete;
	StorageManager(const StorageManager&) = delete;
	StorageManager& operator=(StorageManager&&) = delete;
	StorageManager& operator=(const StorageManager&) = delete;
	~StorageManager();

      private:
};

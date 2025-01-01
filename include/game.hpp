#pragma once

#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

// TODO: Cleanup this
class Game {
      private:
	Game();
	Game(Game&&) = delete;
	Game(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	Game& operator=(const Game&) = delete;

      public:
	~Game();

	static Game* getInstance() {
		static Game instance;
		return &instance;
	}

	void init();
	void save();

	[[nodiscard]] SDL_AppResult iterate();
	[[nodiscard]] SDL_AppResult event(const union SDL_Event& event);

	[[nodiscard]] class LocaleManager* getLocaleManager() const { return mLocaleManager.get(); }
	[[nodiscard]] class SystemManager* getSystemManager() const { return mSystemManager.get(); }

	void setKey(std::size_t key, bool val);
	[[nodiscard]] std::span<bool> getKeystate();

	void setPlayerID(const EntityID id) { mPlayer = id; }
	[[nodiscard]] EntityID getPlayerID() const { return mPlayer; }

	class Level* getLevel() const { return mCurrentLevel.get(); }

      private:
	void gui();

	std::unique_ptr<class EventManager> mEventManager;

	std::unique_ptr<class SystemManager> mSystemManager;
	std::unique_ptr<class LocaleManager> mLocaleManager;

	std::unique_ptr<class Level> mCurrentLevel;

	std::unique_ptr<class StorageManager> mStorageManager;

	std::uint64_t mTicks;
	EntityID mPlayer;
};

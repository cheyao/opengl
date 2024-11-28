#pragma once

#include "managers/entityManager.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

// TODO: Cleanup this
class Game {
	// Our storage manager gotta restore state
	friend class StorageManager;

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

	[[nodiscard]] SDL_AppResult iterate();
	[[nodiscard]] SDL_AppResult event(const union SDL_Event& event);

	[[nodiscard]] class LocaleManager* getLocaleManager() const { return mLocaleManager; }
	[[nodiscard]] class SystemManager* getSystemManager() const { return mSystemManager; }

	[[nodiscard]] inline std::string getBasePath() const { return mBasePath; }

	void setKey(std::size_t key, bool val);
	[[nodiscard]] std::span<bool> getKeystate();

	void setPlayerID(const EntityID id) { mPlayer = id; }
	[[nodiscard]] EntityID getPlayerID() const { return mPlayer; }

	class Level* getLevel() const { return mCurrentLevel; }

      private:
	void gui();

	std::unique_ptr<class EventManager> mEventManager;

	class SystemManager* mSystemManager;
	class LocaleManager* mLocaleManager;
	class StorageManager* mStorageManager;

	std::uint64_t mTicks;
	std::string mBasePath;
	EntityID mPlayer;

	class Level* mCurrentLevel;
};

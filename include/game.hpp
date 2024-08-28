#pragma once

#include "managers/systemManager.hpp"
#include "third_party/Eigen/Core"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

// TODO: Cleanup this
class Game {
	// Our storage manager gotta restore state
	friend class StorageManager;

      public:
	Game();
	Game(Game&&) = delete;
	Game(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	Game& operator=(const Game&) = delete;
	~Game();

	[[nodiscard]] SDL_AppResult iterate();
	[[nodiscard]] SDL_AppResult event(const union SDL_Event& event);

	void setPause(const bool val) { mPaused = val; }
	[[nodiscard]] bool isPaused() const { return mPaused; }

	void setUIScale(const float scale) { mUIScale = scale; }

	[[nodiscard]] class LocaleManager* getLocaleManager() const { return mLocaleManager; }
	[[nodiscard]] class SystemManager* getSystemManager() const { return mSystemManager; }

	[[nodiscard]] inline std::string fullPath(const std::string& path) const {
		return (mBasePath + "assets" SEPARATOR + path);
	}
	[[nodiscard]] inline std::string getBasePath() const { return mBasePath; }

	void setDemensions(const int width, const int height) { mSystemManager->setDemensions(width, height); }
	Eigen::Vector2f getDemensions() const { return mSystemManager->getDemensions(); }

	void setKey(const size_t key, const bool val);
	[[nodiscard]] bool* getKeystate();

	void quit() { mQuit = true; }

      private:
	void gui();

	std::unique_ptr<class EventManager> mEventManager;

	class SystemManager* mSystemManager;
	class LocaleManager* mLocaleManager;
	class StorageManager* mStorageManager;

	float mUIScale;
	uint64_t mTicks;
	std::string mBasePath;

	bool mPaused;
	bool mQuit;

	class Level* mCurrentLevel;
};

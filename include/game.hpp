#pragma once

#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// TODO: Cleanup this
class Game {
      public:
	Game();
	Game(Game&&) = delete;
	Game(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	Game& operator=(const Game&) = delete;
	~Game();

	[[nodiscard]] SDL_AppResult iterate();
	[[nodiscard]] SDL_AppResult event(const union SDL_Event& event);

	[[nodiscard]] int getWidth() const;
	[[nodiscard]] int getHeight() const;

	void setPause(const bool val) { mPaused = val; }
	[[nodiscard]] bool isPaused() const { return mPaused; }

	void setUIScale(const float scale) { mUIScale = scale; }

	[[nodiscard]] class LocaleManager* getLocaleManager() const { return mLocaleManager; }

	[[nodiscard]] inline std::string fullPath(const std::string& path) const {
		return (mBasePath + "assets" SEPARATOR + path);
	}
	[[nodiscard]] std::string getBasePath() const { return mBasePath; }

	void addUI(class UIScreen* ui) { mUI.emplace_back(ui); }
	void removeUI(class UIScreen* ui);
	[[nodiscard]] const std::vector<class UIScreen*>& getUIs() const { return mUI; }
	[[nodiscard]] float getScale() const { return mUIScale; }

	void setDemensions(const int width, const int height);

	void setKey(const size_t key, const bool val);

	void quit() { mQuit = true; }

      private:
	void setup();

	void gui();
	void input();
	void update();
	void draw();

	std::unique_ptr<class EventManager> mEventManager;
	std::unique_ptr<class ManagerManager> mManagerManager;

	class LocaleManager* mLocaleManager;

	std::vector<class UIScreen*> mUI;
	float mUIScale;

	uint64_t mTicks;
	std::string mBasePath;

	bool mPaused;
	bool mQuit;
};

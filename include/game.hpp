#pragma once

#include "managers/localeManager.hpp"
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

	void addActor(class Actor* actor);
	void removeActor(class Actor* actor);

	[[nodiscard]] int getWidth() const;
	[[nodiscard]] int getHeight() const;

	void setPause(const bool val) { mPaused = val; }
	[[nodiscard]] bool isPaused() const { return mPaused; }

	void setUIScale(const float scale) { mUIScale = scale; }

	[[nodiscard]] class Texture* getTexture(const std::string& name, const bool srgb = false);
	[[nodiscard]] class Shader* getShader(const std::string& vert, const std::string& frag,
					      const std::string& geom = "");
	[[nodiscard]] class Renderer* getRenderer() const { return mRenderer; }
	[[nodiscard]] class FontManager* getFontManager() const { return mFontManager; }
	[[nodiscard]] class LocaleManager* getLocaleManager() const { return mLocaleManager; }
	[[nodiscard]] class PhysicsManager* getPhysicsManager() const  { return mPhysicsManager; }

	[[nodiscard]] inline std::string fullPath(const std::string& path) const {
		return (mBasePath + "assets" SEPARATOR + path);
	}

	void addUI(class UIScreen* ui) { mUI.emplace_back(ui); }
	void removeUI(class UIScreen* ui);
	[[nodiscard]] const std::vector<class UIScreen*>& getUIs() const { return mUI; }
	[[nodiscard]] float getScale() const { return mUIScale; }

	void setKey(const size_t key, const bool val);

	void quit() { mQuit = true; }

      private:
	void setup();
	
	void gui();
	void input();
	void update();
	void draw();

	std::unique_ptr<class TextureManager> mTextures;
	std::unique_ptr<class ShaderManager> mShaders;
	std::unique_ptr<class EventManager> mEventManager;
	
	class Renderer* mRenderer;
	class FontManager* mFontManager;
	class LocaleManager* mLocaleManager;
	class PhysicsManager* mPhysicsManager;

	std::vector<class Actor*> mActors;
	std::vector<class Actor*> mPendingActors;
	struct SDL_Mutex* mActorMutex;

	std::vector<class UIScreen*> mUI;
	float mUIScale;

	uint64_t mTicks;
	std::string mBasePath;

	bool mPaused;
	bool mQuit;
};

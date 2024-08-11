#pragma once

#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifdef HOT
#include <filesystem>
#endif

class Game {
      public:
	Game();
	Game(Game&&) = delete;
	Game(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	Game& operator=(const Game&) = delete;
	~Game();

	[[nodiscard]] int iterate();
	[[nodiscard]] int event(const union SDL_Event& event);

	void addActor(class Actor* actor);
	void removeActor(class Actor* actor);

	[[nodiscard]] int getWidth() const;
	[[nodiscard]] int getHeight() const;

	void pause() { mPaused = true; }

	[[nodiscard]] class Texture* getTexture(const std::string& name);
	[[nodiscard]] class Shader* getShader(const std::string& vert, const std::string& frag,
					      const std::string& geom = "");
	[[nodiscard]] class Renderer* getRenderer() { return mRenderer; }

	[[nodiscard]] inline std::string fullPath(const std::string& path) const {
		return (mBasePath + "assets" + SEPARATOR + path);
	};

	void addUI(class UIScreen* ui) { mUI.emplace_back(ui); };
	void removeUI(class UIScreen* ui);
	[[nodiscard]] const std::vector<class UIScreen*>& getUIs() { return mUI; };

	void setKey(const size_t key, const bool val) { mKeys[key] = val; }

      private:
	void input();
	void update();
	void gui();
	void draw();
	void setup();

	std::unique_ptr<class TextureManager> mTextures;
	std::unique_ptr<class ShaderManager> mShaders;

	class Renderer* mRenderer;

	std::vector<class Actor*> mActors;
	std::vector<class Actor*> mPendingActors;
	bool mUpdatingActors;

	std::vector<class UIScreen*> mUI;

	uint64_t mTicks;
	std::string mBasePath;

	bool mPaused;
	bool mVsync;

	bool* mKeys;

#ifdef HOT
	std::filesystem::file_time_type last_time;
#endif
};

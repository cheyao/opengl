#pragma once

#include "utils.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifdef DEBUG
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

	int iterate();
	int event(const union SDL_Event& event);

	void addActor(class Actor* actor);
	void removeActor(class Actor* actor);

	[[nodiscard]] int getWidth() const;
	[[nodiscard]] int getHeight() const;

	void pause() { mPaused = true; }

	class Texture* getTexture(const std::string& name);
	class Shader* getShader(const std::string& vert, const std::string& frag);
	class Renderer* getRenderer() { return mRenderer; }

	inline std::string fullPath(const std::string& path) const { return (mBasePath + "assets" + SEPARATOR + path); };

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

	uint64_t mTicks;
	std::string mBasePath;

	bool mPaused;

#ifdef DEBUG
	std::filesystem::file_time_type last_time;
#endif
};

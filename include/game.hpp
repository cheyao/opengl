#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#define fullPath(path) (mBasePath + "assets" + SEPARATOR + path)

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

	[[nodiscard]] int getWidth() const { return mWidth; }
	[[nodiscard]] int getHeight() const { return mHeight; }

	void setCamera(class CameraComponent* camera) { mCamera = camera; }

	void pause() { mPaused = true; }

	class Texture* getTexture(const std::string& name);

  private:
	void input();
	void update();
	void gui();
	void draw();
	void setup();

	struct SDL_Window* mWindow;
	std::unique_ptr<class GLManager> mGL;
	std::unique_ptr<class TextureManager> mTextures;
	std::unique_ptr<class ShaderManager> mShaders;

	class CameraComponent* mCamera;

	class Model* mModel;

	std::vector<class Actor*> mActors;
	std::vector<class Actor*> mPendingActors;
	bool mUpdatingActors;

	int mWidth, mHeight;
	uint64_t mTicks;
	std::string mBasePath;

	bool mPaused;
	
	class Mesh* mWindowMesh;
	// TODO: Seperate draw class
};

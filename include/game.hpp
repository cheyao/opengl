#pragma once

#include "third_party/Eigen/Geometry"

#include <cstdint>
#include <string>
#include <vector>

#define fullPath(path) (mBasePath + "assets/" + path)

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

	int getWidth() { return mWidth; }
	int getHeight() { return mHeight; }

	void setCamera(class CameraComponent* camera) { mCamera = camera; }

  private:
	void input();
	void update();
	void gui();
	void draw();
	void setup();

	struct SDL_Window* mWindow;
	class GLManager* mGL;
	class TextureManager* mTextures;
	class ShaderManager* mShaders;

	class VertexArray* mVertex;

	class CameraComponent* mCamera;

	std::vector<class Actor*> mActors;
	std::vector<class Actor*> mPendingActors;
	bool mUpdatingActors;

	int mWidth, mHeight;
	uint64_t mTicks;
	std::string mBasePath;
	// TODO: Seperate draw class
};

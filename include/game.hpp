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

	Eigen::Affine3f mView;
	Eigen::Affine3f mProjection;

  private:
	void input();
	void update();
	void gui();
	void draw();
	void setup();

	struct SDL_Window* mWindow;
	class GLManager* mGL;
	class TextureManager* mTextures;

	class Shader* mShader;
	class Shader* mSourceShader;
	class VertexArray* mVertex;

	std::vector<class Actor*> mActors;
	std::vector<class Actor*> mPendingActors;
	bool mUpdatingActors;

	int mWidth, mHeight;
	uint64_t mTicks;
	std::string mBasePath;
	// TODO: Seperate draw class
};

#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"

class Game {
       public:
	Game();
	~Game();

	int init();
	int iterate();
	int event(const SDL_Event& constEvent);

	void addActor(class Actor* actor);
	void removeActor(class Actor* actor);

	void addSprite(class SpriteComponent* sprite);
	void removeSprite(class SpriteComponent* sprite);

	void addHole(class Hole* hole);
	void removeHole(class Hole* hole);
	std::vector<class Hole*>& getHoles() { return mHoles; }

	std::unordered_map<int, bool>& getKeyboard() { return mKeyboard; }
	Mouse& getMouse() { return mMouse; }

	SDL_Texture* getTexture(const std::string& path) {
		return mTextures[path];
	}

	// No copy
	Game(const Game&) = delete;
	Game& operator=(const Game& x) = delete;

	void redraw() { mRedraw = true; }

       private:
	void input();
	void update();
	void gui();
	void draw();
	int loadTexture(const std::string& textureName);

	std::vector<class Actor*> mActors;
	std::vector<class Actor*> mPendingActors;

	std::vector<class SpriteComponent*> mSprites;

	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	Uint64 mTicks;

	bool mUpdatingActors;

	int mWindowWidth, mWindowHeight;

	std::unordered_map<std::string, SDL_Texture*> mTextures;

	std::unordered_map<int, bool> mKeyboard;
	Mouse mMouse;

	std::string mBasePath;

	std::vector<class Hole*> mHoles;

	bool mRedraw;
};


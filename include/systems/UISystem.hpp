#pragma once

#include <vector>

/*
 * The UI system
 *
 * There shall be classes called "Screen" which are the base class that we will draw
 */
class UISystem {
      public:
	explicit UISystem(class Game* game);
	UISystem(UISystem&&) = delete;
	UISystem(const UISystem&) = delete;
	UISystem& operator=(UISystem&&) = delete;
	UISystem& operator=(const UISystem&) = delete;
	~UISystem() = default;

	void update(class Scene* scene, float delta);
	void draw(class Scene* scene);

      private:
	class Game* mGame;
	std::vector<class Screen*> mScreenStack;
};

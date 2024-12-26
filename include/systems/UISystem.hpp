#pragma once

#include <memory>
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

	void addScreen(class Screen* screen) { mScreenStack.emplace_back(screen); }
	void pop() { mScreenStack.pop_back(); }
	class Screen* top() { return mScreenStack.back(); }

	bool empty() const { return mScreenStack.empty(); }
	class Mesh* getMesh() const { return mMesh.get(); }

      private:
	class Game* mGame;
	std::unique_ptr<class Mesh> mMesh;
	std::vector<class Screen*> mScreenStack;
};

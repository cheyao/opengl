#pragma once

#include <vector>

// TODO: Components?
class UIScreen {
      public:
	explicit UIScreen(class Game* game);
	UIScreen(UIScreen&&) = delete;
	UIScreen(const UIScreen&) = delete;
	UIScreen& operator=(UIScreen&&) = delete;
	UIScreen& operator=(const UIScreen&) = delete;
	virtual ~UIScreen();

	class Game* getGame() const { return mGame; }

	void update(const float delta);
	void processInput(const bool* keys);
	void draw(const class Shader* shader);

	enum UIState { Active, Closed };
	void close();
	UIState getState() const { return mState; }

	void addComponent(class UIComponent* component) { mComponents.emplace_back(component); }
	void removeComponent(class UIComponent* component);

      protected:
	class Game* mGame;

	std::vector<class UIComponent*> mComponents;

	UIState mState;
};

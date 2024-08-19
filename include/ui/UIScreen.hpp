#pragma once

#include <SDL3/SDL.h>
#include <vector>

// TODO: Component priority

// NOTE: Component order matters!
class UIScreen {
      public:
	explicit UIScreen(class Game* game, const std::string name);
	UIScreen(UIScreen&&) = delete;
	UIScreen(const UIScreen&) = delete;
	UIScreen& operator=(UIScreen&&) = delete;
	UIScreen& operator=(const UIScreen&) = delete;
	virtual ~UIScreen();

	class Game* getGame() const { return mGame; }

	void update(const float delta);
	virtual void updateScreen(const float delta);
	void processInput(const bool* keys);
	void draw(const class Shader* shader);
	void drawText(const class Shader* shader);
	void touch(const SDL_FingerID& finger, const float x, const float y, const bool lift);

	// TODO: Paused
	enum UIState { ACTIVE, CLOSED, DEAD };
	UIState getState() const { return mState; }
	void setState(const UIState state) { mState = state; }

	const std::string& getName() const { return mName; }

	void addComponent(class UIComponent* component) { mComponents.emplace_back(component); }
	void removeComponent(class UIComponent* component);

      protected:
	class Game* mGame;

	std::vector<class UIComponent*> mComponents;

	UIState mState;

      private:
	const std::string mName;
};

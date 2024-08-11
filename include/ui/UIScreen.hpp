#pragma once

class UIScreen {
      public:
	explicit UIScreen(class Game* game);
	UIScreen(UIScreen&&) = delete;
	UIScreen(const UIScreen&) = delete;
	UIScreen& operator=(UIScreen&&) = delete;
	UIScreen& operator=(const UIScreen&) = delete;
	virtual ~UIScreen();

	class Game* getGame() const { return mGame; }

	virtual void update([[maybe_unused]] float deltaTime);
	virtual void draw([[maybe_unused]] class Shader* shader);
	virtual void processInput([[maybe_unused]] const bool* keys);

	enum UIState { Active, Closed };
	void close();
	UIState getState() const { return mState; }

      protected:
	class Game* mGame;

	UIState mState;
};

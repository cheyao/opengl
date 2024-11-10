#pragma once

class Screen {
      public:
	explicit Screen(class Game* game);
	Screen(Screen&&) = delete;
	Screen(const Screen&) = delete;
	Screen& operator=(Screen&&) = delete;
	Screen& operator=(const Screen&) = delete;
	virtual ~Screen() = default;

	virtual void update(class Scene* scene, float delta);
	virtual void draw(class Scene* scene);

      protected:
	class Game* mGame;
};

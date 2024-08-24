#pragma once

class InputSystem {
      public:
	InputSystem(class Game* game);
	InputSystem(InputSystem&&) = delete;
	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(InputSystem&&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	~InputSystem() = default;

	void update(class Scene* scene, const float delta);

      private:
	class Game* mGame;
};

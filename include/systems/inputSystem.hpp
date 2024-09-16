#pragma once

class InputSystem {
      public:
	InputSystem(class Game* game);
	InputSystem(InputSystem&&) = delete;
	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(InputSystem&&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	~InputSystem() = default;

	// Updates velocity and mouse
	void update(class Scene* scene, const float delta);

      private:
	void updateMouse(class Scene* scene, const float delta);
	void registerClick(const float x, const float y);

	constexpr const static float LONG_PRESS_ACTIVATION_TIME = 0.5f;

	class Game* mGame;

	float mPressLength;
};

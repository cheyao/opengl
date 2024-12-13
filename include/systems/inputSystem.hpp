#pragma once

#include "opengl/mesh.hpp"
#include "third_party/Eigen/Core"

#include <memory>

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
	void draw(class Scene* scene);

	void registerClick(const float x, const float y) {
		mPressLength += 0.000001;
		mPressedX = x;
		mPressedY = y;
	}

      private:
	constexpr const static float LONG_PRESS_ACTIVATION_TIME = 0.1f;

	void updateMouse(class Scene* scene, const float delta);
	void tryPlace(class Scene* scene, const Eigen::Vector2i& pos);

	class Game* mGame;

	float mPressedX;
	float mPressedY;
	float mPressLength;

	std::unique_ptr<class Mesh> mMesh;

	struct {
		bool render;
		class Texture* texture;
		Eigen::Vector2i pos;
	} mDestruction;
};

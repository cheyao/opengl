#pragma once

#include <memory>

class SystemManager {
      public:
	explicit SystemManager(class Game* game);
	SystemManager(SystemManager&&) = delete;
	SystemManager(const SystemManager&) = delete;
	SystemManager& operator=(SystemManager&&) = delete;
	SystemManager& operator=(const SystemManager&) = delete;
	~SystemManager();

	void update();

	void setDemensions(const int width, const int height);

      private:
	class Game* mGame;

	// The order of the systems shall be listed by the order they are updated
	std::unique_ptr<class PhysicsSystem> mPhysicsSystem;
	std::unique_ptr<class CameraSystem> mCameraSystem;
	std::unique_ptr<class RenderSystem> mRenderSystem;
	std::unique_ptr<class TextSystem> mTextSystem;
};

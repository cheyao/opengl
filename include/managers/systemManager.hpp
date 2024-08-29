#pragma once

#include "third_party/Eigen/Core"

#include <memory>
#include <string>

class SystemManager {
      public:
	explicit SystemManager(class Game* game);
	SystemManager(SystemManager&&) = delete;
	SystemManager(const SystemManager&) = delete;
	SystemManager& operator=(SystemManager&&) = delete;
	SystemManager& operator=(const SystemManager&) = delete;
	~SystemManager();

	void update(class Scene* scene, const float delta);

	void setDemensions(const int width, const int height);
	[[nodiscard]] Eigen::Vector2f getDemensions() const;
	[[nodiscard]] class Texture* getTexture(const std::string& name, const bool srgb = false);
	[[nodiscard]] class Shader* getShader(const std::string& vert, const std::string& frag,
					      const std::string& geom = "");

      private:
	void printDebug(class Scene* scene);

	class Game* mGame;

	// The order of the systems shall be listed by the order they are updated
	std::unique_ptr<class PhysicsSystem> mPhysicsSystem;
	std::unique_ptr<class RenderSystem> mRenderSystem;
	std::unique_ptr<class TextSystem> mTextSystem;
	std::unique_ptr<class InputSystem> mInputSystem;
};

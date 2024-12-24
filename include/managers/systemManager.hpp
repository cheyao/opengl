#pragma once

#include "systems/textSystem.hpp"
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
	// Clicks must be event driven
	void registerClick(const float x, const float y);

	[[nodiscard]] Eigen::Vector2f getDemensions() const;
	// Gets texture relative to the asset texture folder
	[[nodiscard]] class Texture* getTexture(const std::string& name, const bool srgb = true);
	[[nodiscard]] class Shader* getShader(const std::string& vert, const std::string& frag,
					      const std::string& geom = "");

	[[nodiscard]] class UISystem* getUISystem() const { return mUISystem.get(); }
	[[nodiscard]] class TextSystem* getTextSystem() const { return mTextSystem.get(); }

      private:
	void printDebug(class Scene* scene);
	void updatePlayer(class Scene* scene);

	class Game* mGame;

	// The order of the systems shall be listed by the order they are updated
	std::unique_ptr<class PhysicsSystem> mPhysicsSystem;
	std::unique_ptr<class RenderSystem> mRenderSystem;
	std::unique_ptr<class InputSystem> mInputSystem;
	std::unique_ptr<class TextSystem> mTextSystem;
	std::unique_ptr<class UISystem> mUISystem;
};

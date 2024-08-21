#pragma once

#include "systems/system.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <memory>
#include <vector>

class RenderSystem : public System {
      public:
	explicit RenderSystem(class Game* game);
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	~RenderSystem();

	[[nodiscard]] inline int getWidth() const { return mWidth; }
	[[nodiscard]] inline int getHeight() const { return mHeight; }
	[[nodiscard]] Eigen::Vector2f getDPI() const;

	void setDemensions(int width, int height);
	void setCamera(class CameraComponent* camera);
	void setDisplayScale() const;

	void addSprite(class DrawComponent* sprite);
	void removeSprite(class DrawComponent* sprite);
	void addPointLight(class Actor* source) { mPointLights.emplace_back(source); };
	void setWindowRelativeMouseMode(int mode) const;
	// TODO: Add more lights

	void draw();
	void reload() const;
	void swapWindow() const;

      private:
	void setLights(class Shader* shader) const;
	void setUIMatrix();

	class Game* mGame;

	// These are pointers managed by RenderSystem
	struct SDL_Window* mWindow; // RAII?
	std::unique_ptr<class GLManager> mGL;
	std::unique_ptr<class Framebuffer> mFramebuffer;
	std::unique_ptr<class UBO> mMatricesUBO;

	std::vector<class DrawComponent*> mDrawables;
	std::vector<class Cubemap*> mCubemaps;
	std::vector<class Actor*> mPointLights;

	int mWidth, mHeight;

	class CameraComponent* mCamera;
};

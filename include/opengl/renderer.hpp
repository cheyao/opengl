#pragma once

#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <memory>
#include <vector>

class Renderer {
      public:
	explicit Renderer(class Game* game);
	Renderer(Renderer&&) = delete;
	Renderer(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	~Renderer();

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
	void setUIMatrix() const;

	class Game* mGame;

	// These are pointers managed by renderer
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

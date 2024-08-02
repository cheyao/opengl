#pragma once

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

	void setDemensions(int width, int height);
	void setCamera(class CameraComponent* camera) { mCamera = camera; }

	void addSprite(class DrawComponent* sprite);
	void removeSprite(class DrawComponent* sprite);
	// TODO: Add lights
	// void addLight(class Actor* source);

	void draw() const;
	void reload() const;

  private:
	void setLights(class Shader* shader) const;

	class Game* mOwner;

	struct SDL_Window* mWindow;
	std::unique_ptr<class GLManager> mGL;
	std::unique_ptr<class Framebuffer> mFramebuffer;

	std::vector<class DrawComponent*> mDrawables;
	std::vector<class Cubemap*> mCubemaps;

	int mWidth, mHeight;

	class CameraComponent* mCamera;
};

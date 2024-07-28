#pragma once

#include <memory>

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

	void draw(class CameraComponent* camera);

  private:
	class Game* mOwner;

	struct SDL_Window* mWindow;
	std::unique_ptr<class GLManager> mGL;
	std::unique_ptr<class Framebuffer> mFramebuffer;

	int mWidth, mHeight;

	std::unique_ptr<class Mesh> mWindowMesh;
	std::unique_ptr<class Model> mModel;
};

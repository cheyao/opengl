#pragma once

#include <SDL3/SDL_video.h>
#include <memory>
#include <string>

// TODO: DPI
class RenderSystem {
      public:
	explicit RenderSystem(class Game* game);
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	~RenderSystem();

	[[nodiscard]] inline int getWidth() const { return mWidth; }
	[[nodiscard]] inline int getHeight() const { return mHeight; }

	[[nodiscard]] class Texture* getTexture(const std::string& name, const bool srgb = false);
	[[nodiscard]] class Shader* getShader(const std::string& vert, const std::string& frag,
					      const std::string& geom = "");

	void setDemensions(int width, int height);

	void draw(class Scene* scene);
	void reload() const;
	void swapWindow() const;
	void present() const;

      private:
	void setLights(class Shader* shader) const;
	void setOrtho() const;
	void setPersp() const;
	void drawHUD(class Scene* scene);

	class Game* mGame;

	// These are pointers managed by RenderSystem
	std::unique_ptr<struct SDL_Window, void (*)(SDL_Window*)> mWindow;
	std::unique_ptr<struct SDL_Cursor, void (*)(SDL_Cursor*)> mCursor;
	std::unique_ptr<struct SDL_Surface, void (*)(SDL_Surface*)> mIcon;

	std::unique_ptr<class GLManager> mGL;
	std::unique_ptr<class Framebuffer> mFramebuffer;
	std::unique_ptr<class UBO> mMatricesUBO;
	std::unique_ptr<class TextureManager> mTextures;
	std::unique_ptr<class ShaderManager> mShaders;

	std::unique_ptr<class Mesh> mMesh;

	int mWidth, mHeight;
};

#pragma once

#include <memory>

class Framebuffer {
public:
	explicit Framebuffer(class Game* owner);
	Framebuffer(Framebuffer &&) = delete;
	Framebuffer(const Framebuffer &) = delete;
	Framebuffer &operator=(Framebuffer &&) = delete;
	Framebuffer &operator=(const Framebuffer &) = delete;
	~Framebuffer();

	void setDemensions(int width, int height);
	void swap(struct SDL_Window* window);

private:
	class Game* mOwner;

	unsigned int mRBO;
	unsigned int mScreen;
	unsigned int mScreenTexture;

	std::unique_ptr<class Mesh> mScreenMesh;
};

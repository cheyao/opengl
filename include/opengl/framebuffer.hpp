#pragma once

#include "third_party/glad/glad.h"

#include <memory>

class Framebuffer {
  public:
	explicit Framebuffer(class RenderSystem* owner);
	Framebuffer(Framebuffer&&) = delete;
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(Framebuffer&&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	~Framebuffer();

	void setDemensions(const int width, const int height);
	void swap();
	void bind() const;

  private:
	class RenderSystem* mOwner;

	GLuint mScreen;
	GLuint mScreenTexture;

	std::unique_ptr<class Mesh> mScreenMesh;
};

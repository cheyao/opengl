#include "opengl/framebuffer.hpp"

#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "systems/renderSystem.hpp"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#endif

Framebuffer::Framebuffer(RenderSystem* owner) : mOwner(owner), mRBO(0), mScreen(0), mScreenTexture(0) {
	SDL_Log("Framebuffer.cpp: Generating framebuffer");

	SDL_assert(glGenFramebuffers != nullptr);
	SDL_assert(glGenRenderbuffers != nullptr);

	glGenFramebuffers(1, &mScreen);
	glBindFramebuffer(GL_FRAMEBUFFER, mScreen);

	glGenTextures(1, &mScreenTexture);
	glBindTexture(GL_TEXTURE_2D, mScreenTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mScreenTexture, 0);

	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);

	[[unlikely]] if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Framebuffer is not complete");
		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have enough memory");

		throw std::runtime_error("Framebuffer.cpp: Failed to create framebuffer");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mScreen);

	constexpr const static float pos[] = {
		-1.0f, +1.0f, 0.0f, // Top left
		-1.0f, -1.0f, 0.0f, // Bot left
		+1.0f, +1.0f, 0.0f, // Top right
		+1.0f, -1.0f, 0.0f, // Bot right
	};
	constexpr const static unsigned int indices[] = {0, 1, 2, 3, 2, 1};

	Shader* mShader = mOwner->getShader("framebuffer.vert", "framebuffer.frag");
	mShader->activate();
	mShader->set("screen"_u, 0);

	glActiveTexture(GL_TEXTURE0);

	mScreenMesh = std::make_unique<Mesh>(pos, std::span<float>(), std::span<float>(), indices);
}

void Framebuffer::setDemensions(const int width, const int height) {
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindTexture(GL_TEXTURE_2D, mScreenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::swap() {
	GLint mode[2] = {};
	if (glPolygonMode) {
		glGetIntegerv(GL_POLYGON_MODE, mode);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef DEBUG
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	glDisable(GL_DEPTH_TEST);

	Shader* mShader = mOwner->getShader("framebuffer.vert", "framebuffer.frag");
	mShader->activate();

#ifdef GLES
	mShader->set("gamma"_u, false);
#else
	mShader->set("gamma"_u, true);
#endif

	glBindTexture(GL_TEXTURE_2D, mScreenTexture);

	mScreenMesh->draw(mShader);

#ifdef IMGUI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

	mOwner->swapWindow();

	glBindFramebuffer(GL_FRAMEBUFFER, mScreen);
	glEnable(GL_DEPTH_TEST);

	if (glPolygonMode) {
		glPolygonMode(GL_FRONT_AND_BACK, mode[0]);
	}
}

Framebuffer::~Framebuffer() {
	glDeleteTextures(1, &mScreenTexture);
	glDeleteFramebuffers(1, &mScreen);
	glDeleteRenderbuffers(1, &mRBO);
}

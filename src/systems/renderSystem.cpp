#include "systems/renderSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/glManager.hpp"
#include "managers/shaderManager.hpp"
#include "managers/textureManager.hpp"
#include "opengl/framebuffer.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/ubo.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "ui/UIScreen.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cassert>
#include <memory>

#ifdef IMGUI
#ifdef GLES
#define IMGUI_IMPL_OPENGL_ES3
#endif
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

// Hacks to get browser width and hight
EM_JS(int, browserHeight, (), { return window.innerHeight; });
EM_JS(int, browserWidth, (), { return window.innerWidth; });
#endif

RenderSystem::RenderSystem(Game* game)
	: mGame(game), mWindow(nullptr), mGL(nullptr), mFramebuffer(nullptr), mMatricesUBO(nullptr),
	  mTextures(std::make_unique<TextureManager>(mGame->getBasePath())),
	  mShaders(std::make_unique<ShaderManager>(mGame->getBasePath())), mWidth(0), mHeight(0) {
	const SDL_DisplayMode* const DM = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

	SDL_Log("\n");
	SDL_Log("Display info:");
	SDL_Log("Display size: %dx%d", DM->w, DM->h);
	SDL_Log("Display format: %#010x", DM->format);
	SDL_Log("Refresh rate: %f", DM->refresh_rate);
	SDL_Log("\n");

	// Note: These must be set before the window is created https://wiki.libsdl.org/SDL3/SDL_GLattr
#ifdef GLES
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
	// Dunno why but apple want's this
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
#endif

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// NOTE: Don't set emscripten here, it breakes stuff, idk why
#ifdef ANDROID
	mWidth = DM->h;
	mHeight = DM->w;
#else
	mWidth = 1024;
	mHeight = 768;
#endif

	mWindow = SDL_CreateWindow("OpenGL", mWidth, mHeight,
				   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
#ifdef ANDROID
					   | SDL_WINDOW_FULLSCREEN
#endif
#ifdef HIDPI
					   | SDL_WINDOW_HIGH_PIXEL_DENSITY
#endif

	);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Game.cpp: Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with your system/SDL installation");

		throw std::runtime_error("Game.cpp: Failed to create SDL window");
	}

#ifdef __EMSCRIPTEN__
	mWidth = browserWidth();
	mHeight = browserHeight();

	SDL_SetWindowSize(mWindow, mWidth, mHeight);
#endif

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__)
	SDL_SetWindowMinimumSize(mWindow, 480, 320);
#endif

	mGL = std::make_unique<GLManager>(mWindow);

	assert(mGL->getContext() != nullptr);

#ifdef IMGUI
	SDL_Log("Initializing ImGUI");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
	io.IniFilename = nullptr;
#else
	// TODO: Storage path
#endif
#ifdef __ANDROID__
	io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
#endif

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
#ifndef GLES
	ImGui_ImplOpenGL3_Init("#version 410 core");
#else
	ImGui_ImplOpenGL3_Init("#version 300 es  ");
#endif

	SDL_Log("Finished Initializing ImGUI");
#endif

	setDisplayScale();

	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);

	mFramebuffer = std::make_unique<Framebuffer>(this);
	// NOTE: Uncomment if testing framebuffer module
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Matrix uniform
	mMatricesUBO = std::make_unique<UBO>(2 * sizeof(Eigen::Affine3f));
	mMatricesUBO->bind(0);
	setUIMatrix();

	// Debug Info
	mGL->printInfo();

	setOrtho();

	constexpr const static float vertices[] = {
		0.0f, 0.0f, 0.0f, // TL
		0.0f, 1.0f, 0.0f, // BR
		1.0f, 0.0f, 0.0f, // TR
		1.0f, 1.0f, 0.0f  // BL
	};

	constexpr const static float texturePos[] = {
		0.0f, 1.0f, // TR
		0.0f, 0.0f, // BR
		1.0f, 1.0f, // TL
		1.0f, 0.0f  // BL
	};

	const static GLuint indices[] = {2, 1, 0,  // a
					 1, 2, 3}; // b

	mMesh = std::unique_ptr<Mesh>(new Mesh(vertices, {}, texturePos, indices, {}));
}

RenderSystem::~RenderSystem() { SDL_DestroyWindow(mWindow); /* Other stuff are smart pointers */ }

void RenderSystem::setDemensions(int width, int height) {
	mWidth = width;
	mHeight = height;

	int w, h;
	SDL_GetWindowSizeInPixels(mWindow, &w, &h);

	glViewport(0, 0, w, h);
	mFramebuffer->setDemensions(w, h);

	setUIMatrix();

	setOrtho();
}

void RenderSystem::draw(Scene* scene) {
#ifdef DEBUG
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	glClear(GL_DEPTH_BUFFER_BIT);
#endif

	/*
		for (const auto& sprite : mDrawables) {
			Shader* const shader = sprite->getShader();
			shader->activate();

			setLights(shader);
			sprite->draw();
		}
	*/

	for (const auto& [entity, texture, position] :
	     scene->view<Components::texture, Components::position>().each()) {
		(void)entity;
	}

	glDisable(GL_DEPTH_TEST);

	Shader* UIshader = this->getShader("ui.vert", "ui.frag");
	Shader* textShader = this->getShader("text.vert", "text.frag");
	for (const auto& ui : mGame->getUIs()) {
		if (ui->getState() == UIScreen::ACTIVE) {
			ui->draw(UIshader);
			ui->drawText(textShader);
		}
	}

#ifdef __EMSCRIPTEN__
	// Emscripten, SDL3 doesn't correctly report resize atm
	if (browserWidth() != mWidth || browserHeight() != mHeight) {
		mWidth = browserWidth();
		mHeight = browserHeight();

		SDL_SetWindowSize(mWindow, mWidth, mHeight);
	}
#endif
}

void RenderSystem::present() const { mFramebuffer->swap(); }

void RenderSystem::swapWindow() const { SDL_GL_SwapWindow(mWindow); }

void RenderSystem::setUIMatrix() {
	Eigen::Affine3f ortho = Eigen::Affine3f::Identity();

	constexpr const static float left = 0.0f;
	constexpr const static float bottom = 0.0f;
	constexpr const static float near = 0.1f;
	constexpr const static float far = 100.0f;

	const float right = mWidth;
	const float top = mHeight;

	ortho(0, 0) = 2.0f / (right - left);
	ortho(1, 1) = 2.0f / (top - bottom);
	ortho(2, 2) = -2.0f / (far - near);
	ortho(3, 3) = 1;

	ortho(0, 3) = -(right + left) / (right - left);
	ortho(1, 3) = -(top + bottom) / (top - bottom);
	ortho(2, 3) = -(far + near) / (far - near);

	Shader* const UIshader = this->getShader("ui.vert", "ui.frag");
	UIshader->activate();
	UIshader->set("proj", ortho);

	Shader* const textshader = this->getShader("text.vert", "text.frag");
	textshader->activate();
	textshader->set("proj", ortho);
}

void RenderSystem::setDisplayScale() const {
	assert(mWindow != nullptr);

	float scale = SDL_GetWindowDisplayScale(mWindow);
	if (scale <= 0.0f) {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get display context scale: %s\033[0m",
			     SDL_GetError());

		scale = 1.0f;
	}

	mGame->setUIScale(scale);

#ifdef IMGUI
	ImGuiIO& io = ImGui::GetIO();

	ImFont* font =
		io.Fonts->AddFontFromFileTTF(mGame->fullPath("fonts" SEPARATOR "NotoSans.ttf").data(), 16.0f * scale);
	assert(font != NULL);

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
#endif
}

Texture* RenderSystem::getTexture(const std::string& name, const bool srgb) { return mTextures->get(name, srgb); }

Shader* RenderSystem::getShader(const std::string& vert, const std::string& frag, const std::string& geom) {
	return mShaders->get(vert, frag, geom);
}

// NOTE: Not needed for our 2D game

/*
 * void RenderSystem::view() {
 * 	Eigen::Matrix3f R;
 * 	const Eigen::Vector3f up(0.0f, 1.0f, 0.0f);
 * 	R.col(2) = (-mOwner->getForward() * 100.f).normalized();
 * 	R.col(0) = up.cross(R.col(2)).normalized();
 * 	R.col(1) = R.col(2).cross(R.col(0));
 *
 * 	Eigen::Affine3f mViewMatrix = Eigen::Affine3f::Identity();
 * 	mViewMatrix.matrix().topLeftCorner<3, 3>() = R.transpose();
 * 	mViewMatrix.matrix().topRightCorner<3, 1>() = -R.transpose(); // * mOwner->getPosition();
 * 	mViewMatrix(3, 3) = 1.0f;
 * }
 */

void RenderSystem::setPersp() const {
	constexpr const static float near = 0.1f;
	constexpr const static float far = 100.0f;
	constexpr const static float range = far - near;
	constexpr const static float fov = 45.0f;

	const float aspect = static_cast<float>(mWidth) / mHeight;
	const float theta = fov * 0.5;
	const float invtan = 1.0f / tan(theta);

	// https://www.songho.ca/opengl/gl_projectionmatrix.html
	Eigen::Affine3f projectionMatrix = Eigen::Affine3f::Identity();
	projectionMatrix(0, 0) = invtan / aspect;
	projectionMatrix(1, 1) = invtan;
	projectionMatrix(2, 2) = -(near + far) / range;
	projectionMatrix(3, 2) = -1;
	projectionMatrix(2, 3) = -2 * near * far / range;
	projectionMatrix(3, 3) = 0;

	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), projectionMatrix);
}

void RenderSystem::setOrtho() const {
	constexpr const static float left = 0.0f;
	constexpr const static float bottom = 0.0f;
	constexpr const static float near = 0.0f;
	constexpr const static float far = 1000.0f;

	const float right = static_cast<float>(mWidth);
	const float top = static_cast<float>(mHeight);

	Eigen::Affine3f projectionMatrix = Eigen::Affine3f::Identity();
	projectionMatrix(0, 0) = 2.0f / (right - left);
	projectionMatrix(1, 1) = 2.0f / (top - bottom);
	projectionMatrix(2, 2) = -2.0f / (far - near);
	projectionMatrix(3, 3) = 1;
	projectionMatrix(0, 3) = -(right + left) / (right - left);
	projectionMatrix(1, 3) = -(top + bottom) / (top - bottom);
	projectionMatrix(2, 3) = -(far + near) / (far - near);

	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), projectionMatrix);
}

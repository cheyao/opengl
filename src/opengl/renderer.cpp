#include "opengl/renderer.hpp"

#include "actors/actor.hpp"
#include "components/cameraComponent.hpp"
#include "components/drawComponent.hpp"
#include "game.hpp"
#include "managers/glManager.hpp"
#include "opengl/framebuffer.hpp"
#include "opengl/shader.hpp"
#include "opengl/ubo.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "ui/UIScreen.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
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

Renderer::Renderer(Game* game)
	: mGame(game), mWindow(nullptr), mGL(nullptr), mFramebuffer(nullptr), mWidth(0), mHeight(0), mCamera(nullptr) {
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
	// Used to force hardware accell:
	// SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

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

	mFramebuffer = std::make_unique<Framebuffer>(mGame);
	// NOTE: Uncomment if testing framebuffer module
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Matrix uniform
	mMatricesUBO = std::make_unique<UBO>(2 * sizeof(Eigen::Affine3f));
	mMatricesUBO->bind(0);
	setUIMatrix();

	// Misc
	mGL->printInfo();

	assert(mGL->getContext() != nullptr);
}

/* NOTE:
 * The uniform matrix object is initialized as following:
 * The uniform is bind eveytime a drawable is added
 * The projection matrix is reset everytime the camera is changed and the window is changed
 * The view matrix is changed once at the start of every frame
 */

Renderer::~Renderer() { SDL_DestroyWindow(mWindow); /* Other stuff are smart pointers */ }

void Renderer::setDemensions(int width, int height) {
	mWidth = width;
	mHeight = height;

	int w, h;
	SDL_GetWindowSizeInPixels(mWindow, &w, &h);

	glViewport(0, 0, w, h);
	mFramebuffer->setDemensions(w, h);

	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), mCamera->getProjectionMatrix());

	setUIMatrix();
}

void Renderer::draw() {
#ifdef DEBUG
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	glClear(GL_DEPTH_BUFFER_BIT);
#endif

	assert(mCamera != nullptr && "Did you forget to uncomment `new Player(this)`?");

	mMatricesUBO->set(1 * sizeof(Eigen::Affine3f), mCamera->getViewMatrix());

	for (const auto& sprite : mDrawables) {
		Shader* const shader = sprite->getShader();
		shader->activate();
		// shader->set("viewPos", mCamera->getOwner()->getPosition());

		setLights(shader);
		sprite->draw();
	}

	glDisable(GL_DEPTH_TEST);

	Shader* UIshader = mGame->getShader("ui.vert", "ui.frag");
	Shader* textShader = mGame->getShader("text.vert", "text.frag");
	for (const auto& ui : mGame->getUIs()) {
		if (ui->getState() == UIScreen::ACTIVE) {
			ui->draw(UIshader);
			ui->drawText(textShader);
		}
	}

	mFramebuffer->swap();

#ifdef __EMSCRIPTEN__
	// Emscripten, SDL3 doesn't correctly report resize atm
	if (browserWidth() != mWidth || browserHeight() != mHeight) {
		mWidth = browserWidth();
		mHeight = browserHeight();

		SDL_SetWindowSize(mWindow, mWidth, mHeight);
	}
#endif
}

void Renderer::swapWindow() const { SDL_GL_SwapWindow(mWindow); }

void Renderer::reload() const {
	for (const auto& sprite : mDrawables) {
		sprite->reload();
	}
}

void Renderer::setCamera(class CameraComponent* camera) {
	assert(camera != nullptr && "Forgot to init camera?");

	mCamera = camera;

	// The projection matrix never gets updated except when the camera is set
	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), mCamera->getProjectionMatrix());
}

void Renderer::addSprite(DrawComponent* sprite) {
	sprite->getShader()->bind("Matrices", 0);

	// Preserve order
	int order = sprite->getDrawOrder();
	auto iter = mDrawables.begin();
	for (; iter != mDrawables.end(); ++iter) {
		if (order < (*iter)->getDrawOrder()) {
			break;
		}
	}

	mDrawables.insert(iter, sprite);
}

void Renderer::removeSprite(DrawComponent* sprite) {
	auto iter = std::find(mDrawables.begin(), mDrawables.end(), sprite);
	mDrawables.erase(iter);
}

void Renderer::setLights(Shader* shader) const {
	// TODO: Concat all these
	(void)shader;
}

void Renderer::setWindowRelativeMouseMode(SDL_bool mode) const {
	if (SDL_SetWindowRelativeMouseMode(mWindow, mode) != 0) {
		SDL_Log("Failed to set relative mouse mode: %s", SDL_GetError());
	}
}

void Renderer::setUIMatrix() {
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

	Shader* const UIshader = mGame->getShader("ui.vert", "ui.frag");
	UIshader->activate();
	UIshader->set("proj", ortho);

	Shader* const textshader = mGame->getShader("text.vert", "text.frag");
	textshader->activate();
	textshader->set("proj", ortho);
}

[[nodiscard]] Eigen::Vector2f Renderer::getDPI() const {
	/*
	int winx, winy;
	SDL_GetWindowSizeInPixels(mWindow, &winx, &winy);
	float scale = SDL_GetWindowDisplayScale(mWindow);
	if (scale == 0.0f) {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get display context scale: %s\033[0m",
			     SDL_GetError());

		scale = 1.0f;
	}
	*/

	return Eigen::Vector2f::Zero();
}

void Renderer::setDisplayScale() const {
	assert(mWindow != nullptr);

	float scale = SDL_GetWindowDisplayScale(mWindow);
	if (scale == 0.0f) {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get display context scale: %s\033[0m",
			     SDL_GetError());

		scale = 1.0f;
	}

	mGame->setUIScale(scale);

#ifdef IMGUI
	ImGuiIO& io = ImGui::GetIO();

	ImFont* font =
		io.Fonts->AddFontFromFileTTF(mGame->fullPath("fonts" SEPARATOR "NotoSans.ttf").data(), 16.0f * scale);
	IM_ASSERT(font != NULL);

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
#endif
}

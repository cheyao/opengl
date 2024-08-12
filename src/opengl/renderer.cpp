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
	mGL = std::make_unique<GLManager>();
	const SDL_DisplayMode* const DM = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

	SDL_Log("\n");
	SDL_Log("Display info:");
	SDL_Log("Display size: %dx%d", DM->w, DM->h);
	SDL_Log("Display format: %#010x", DM->format);
	SDL_Log("Refresh rate: %f", DM->refresh_rate);
	SDL_Log("\n");

#ifdef ANDROID
	mWidth = DM->h;
	mHeight = DM->w;
	/*
#elif defined(__EMSCRIPTEN__)
	mWidth = browserWidth();
	mHeight = browserHeight();
	*/
#else
	mWidth = 1024;
	mHeight = 768;
#endif

	mWindow = SDL_CreateWindow("OpenGL", mWidth, mHeight,
				   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
#ifdef ANDROID
					   | SDL_WINDOW_FULLSCREEN
#endif
	);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Game.cpp: Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with "
			  "your system/SDL installation");

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

	mGL->bindContext(mWindow);

#ifdef IMGUI
	SDL_Log("Initializing ImGUI");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
#if defined(__EMSCRIPTEN__) || defined(ANDROID)
	io.IniFilename = nullptr;
#else
	// TODO: Storage path
#endif

	ImGui::StyleColorsDark();

	SDL_Log("Finished Initializing ImGUI");
#endif

	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);

	glViewport(0, 0, mWidth, mHeight);

#ifdef IMGUI
	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
#ifndef GLES
	ImGui_ImplOpenGL3_Init("#version 410 core");
#else
	ImGui_ImplOpenGL3_Init("#version 300 es");
#endif
#endif

	mGL->printInfo();

	mFramebuffer = std::make_unique<Framebuffer>(mGame);

	// Matrix uniform
	mMatricesUBO = std::make_unique<UBO>(2 * sizeof(Eigen::Affine3f));
	mMatricesUBO->bind(0);

	// NOTE: Uncomment if testing framebuffer module
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// SDL_SetWindowRelativeMouseMode(mWindow, 1);

	// Ortho for UI
	Eigen::Affine3f ortho = Eigen::Affine3f::Identity();

	// ortho(float left, float right, float bottom, float top)
	// glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
	constexpr const static float left = 0.0f;
	const float right = mWidth;
	const float top = mHeight;
	constexpr const static float bottom = 0.0f;
	constexpr const static float near = 0.1f;
	constexpr const static float far = 100.0f;

	ortho(0, 0) = 2.0f / (right - left);
	ortho(1, 1) = 2.0f / (top - bottom);
	ortho(2, 2) = -2.0f / (far - near);
	ortho(3, 3) = 1;

	ortho(0, 3) = -(right + left) / (right - left);
	ortho(1, 3) = -(top + bottom) / (top - bottom);
	ortho(2, 3) = -(far + near) / (far - near);
	// (y, x)

	Shader* const UIshader = mGame->getShader("ui.vert", "ui.frag");
	UIshader->activate();
	UIshader->set("proj", ortho);
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

	glViewport(0, 0, width, height);
	mFramebuffer->setDemensions(width, height);

	mCamera->project();
	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), mCamera->getProjectionMatrix());

	Eigen::Affine3f ortho = Eigen::Affine3f::Identity();

	constexpr const static float left = 0.0f;
	const float right = mWidth;
	const float top = mHeight;
	constexpr const static float bottom = 0.0f;
	constexpr const static float near = 0.1f;
	constexpr const static float far = 100.0f;

	ortho(0, 0) = 2.0f / (right - left);
	ortho(1, 1) = 2.0f / (top - bottom);
	ortho(2, 2) = -2.0f / (far - near);
	ortho(3, 3) = 1;

	ortho(0, 3) = -(right + left) / (right - left);
	ortho(1, 3) = -(top + bottom) / (top - bottom);
	ortho(2, 3) = -(far + near) / (far - near);
	// (y, x)

	Shader* const UIshader = mGame->getShader("ui.vert", "ui.frag");
	UIshader->activate();
	UIshader->set("proj", ortho);
}

void Renderer::draw() {
#ifdef DEBUG
	glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	glClear(GL_DEPTH_BUFFER_BIT);
#endif

	assert(mCamera != nullptr && "Did you forget to uncomment `new Player(this)`?");

	mMatricesUBO->set(1 * sizeof(Eigen::Affine3f), mCamera->getViewMatrix());

	for (const auto& sprite : mDrawables) {
		Shader* const shader = sprite->getShader();
		shader->activate();
		shader->set("viewPos", mCamera->getOwner()->getPosition());

		setLights(shader);
		sprite->draw();
	}

	glDisable(GL_DEPTH_TEST);

	Shader* const UIshader = mGame->getShader("ui.vert", "ui.frag");
	for (const auto& ui : mGame->getUIs()) {
		ui->draw(UIshader);
	}

#ifdef IMGUI
	ImGui::Render();
#endif

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

	// shader->set("dirLight.direction", -0.2f, -1.0f, -0.3f);
	// shader->set("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	// shader->set("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	// shader->set("dirLight.specular", 0.5f, 0.5f, 0.5f);

	/*
#ifdef __cpp_lib_ranges_enumerate
	// Duh libc++ doesn't support enumerate
	for (const auto& [place, value] : std::views::enumerate(mPointLights)) {
#else
	for (size_t place = 0; place < mPointLights.size(); ++place) {
		const auto& value = mPointLights[place];
#endif

		std::string num = "pointLights[";
		num.append(std::to_string(place));

		constexpr float a = 0.7f;
		constexpr float d = 2.8f;
		constexpr float s = 1.0f;

		shader->set(num + "].position", value->getPosition());
		shader->set(num + "].ambient", a, a, a);
		shader->set(num + "].diffuse", d, d, d);
		shader->set(num + "].specular", s, s, s);
		shader->set(num + "].constant", constant);
		shader->set(num + "].linear", linear);
		shader->set(num + "].quadratic", quadratic);
	}
	*/
	const auto& value = mPointLights[0];

	constexpr static const float a = 0.7f;
	constexpr static const float d = 2.8f;
	constexpr static const float s = 1.0f;
	constexpr static const float constant = 1.0f;
	constexpr static const float linear = 0.05f;
	constexpr static const float quadratic = 0.032f;

	// PERF: Only set once - redunant setters
	shader->set("pointLight.position", value->getPosition());
	shader->set("pointLight.ambient", a, a, a * 0.7);
	shader->set("pointLight.diffuse", d, d, d * 0.7);
	shader->set("pointLight.specular", s, s, s * 0.7);
	shader->set("pointLight.constant", constant);
	shader->set("pointLight.linear", linear);
	shader->set("pointLight.quadratic", quadratic);

	// shader->set("spotLight.position", mCamera->getOwner()->getPosition());
	// shader->set("spotLight.direction", mCamera->getOwner()->getForward());
	// shader->set("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	// shader->set("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	// shader->set("spotLight.specular", 1.0f, 1.0f, 1.0f);
	// shader->set("spotLight.constant", 1.0f);
	// shader->set("spotLight.linear", 0.09f);
	// shader->set("spotLight.quadratic", 0.032f);
	// shader->set("spotLight.cutOff", cos(toRadians(12.5f)));
	// shader->set("spotLight.outerCutOff", cos(toRadians(15.0f)));
}

void Renderer::setWindowRelativeMouseMode(SDL_bool mode) const { SDL_SetWindowRelativeMouseMode(mWindow, mode); }

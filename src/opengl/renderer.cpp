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
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_stdinc.h>
#include <cassert>
#include <memory>
#include <string>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

// Hacks to get browser width and hight
EM_JS(int, browserHeight, (), { return window.innerHeight; });
EM_JS(int, browserWidth, (), { return window.innerWidth; });
#endif

Renderer::Renderer(Game* game)
	: mOwner(game), mWindow(nullptr), mGL(nullptr), mFramebuffer(nullptr), mWidth(0), mHeight(0), mCamera(nullptr) {
	mGL = std::make_unique<GLManager>();

	mWindow = SDL_CreateWindow("OpenGL", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Game.cpp: Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with "
			  "your system/SDL installation");

		throw std::runtime_error("Game.cpp: Failed to create SDL window");
	}

#ifdef __EMSCRIPTEN__
	// Hacks for EMSCRIPTEN Full screen
	mWidth = browserWidth();
	mHeight = browserHeight();

	SDL_SetWindowSize(mWindow, mWidth, mHeight);
#else
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
#ifdef __EMSCRIPTEN__
	io.IniFilename = nullptr;
#endif

	ImGui::StyleColorsDark();

	SDL_Log("Finished Initializing ImGUI");
#endif

	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);

	glViewport(0, 0, mWidth, mHeight);

#ifdef IMGUI
	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
	ImGui_ImplOpenGL3_Init("#version 410");
#endif

	mGL->printInfo();

	mFramebuffer = std::make_unique<Framebuffer>(mOwner);

	// Matrix uniform
	mMatricesUBO = std::make_unique<UBO>(2 * sizeof(Eigen::Affine3f));
	mMatricesUBO->bind(0);

	// NOTE: Uncomment if testing framebuffer module
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// SDL_SetWindowRelativeMouseMode(mWindow, 1);
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
}

static float constant = 1.0f;
static float linear = 0.09f;
static float quadratic = 0.032f;

void Renderer::draw() const {
#ifdef IMGUI
	ImGui::Begin("Main Menu");

	ImGui::SliderFloat("Constant", &constant, -1.0f, 10.0f);
	ImGui::SliderFloat("Linear", &linear, -1.0f, 10.0f);
	ImGui::SliderFloat("Quadratic", &quadratic, -1.0f, 10.0f);

	// ImGui::SliderFloat("Ambient", &a, 0.0f, 10.0f);
	// ImGui::SliderFloat("Diffuse", &d, 0.0f, 10.0f);
	// ImGui::SliderFloat("Specular", &s, 0.0f, 10.0f);

	ImGui::End();
#endif

	glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	assert(mCamera != nullptr && "Did you forget to uncomment `new Player(this)`?");

	mMatricesUBO->set(1 * sizeof(Eigen::Affine3f), mCamera->getViewMatrix());

	for (const auto& sprite : mDrawables) {
		Shader* const shader = sprite->getShader();
		shader->activate();
		shader->set("viewPos", mCamera->getOwner()->getPosition()); // Bruh how come I forgot
		shader->set("time", static_cast<float>(SDL_GetTicks()) / 1000);

		setLights(shader);

		sprite->draw();
	}

#ifdef IMGUI
	ImGui::Render();
#endif

	mFramebuffer->swap(mWindow);

#ifdef __EMSCRIPTEN__
	// Emscripten, SDL3 doesn't correctly report resize atm
	if (browserWidth() != mWindowWidth || browserHeight() != mWindowHeight) {
		mWindowWidth = browserWidth();
		mWindowHeight = browserHeight();

		SDL_SetWindowSize(mWindow, mWindowWidth, mWindowHeight);
	}
#endif
}

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
	const Eigen::Vector4f lightPos(1.2f, 1.0f, 2.0f, 1.0f);

	// shader->set("dirLight.direction", -0.2f, -1.0f, -0.3f);
	// shader->set("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	// shader->set("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	// shader->set("dirLight.specular", 0.5f, 0.5f, 0.5f);

#ifdef __cpp_lib_ranges_enumerate
	// Duh libc++ doesn't support enumerate
	for (const auto& [place, value] : std::views::enumerate(mPointLights)) {
#else
	for (size_t place = 0; place < mPointLights.size(); ++place) {
		const auto& value = mPointLights[place];
#endif

		// TODO: Better
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

void Renderer::setWindowRelativeMouseMode(SDL_bool mode) {
	SDL_SetWindowRelativeMouseMode(mWindow, mode);
	SDL_GetWindowRelativeMouseMode(mWindow);
	if (mode) {
		SDL_ShowCursor();
	} else {
		SDL_HideCursor();
	}
}

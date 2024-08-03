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

#include <memory>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

EM_JS(int, browserHeight, (), { return window.innerHeight; });
EM_JS(int, browserWidth, (), { return window.innerWidth; });
EM_JS(int, canvasResize, (), {
	canvas.width = window.innerWidth;
	canvas.height = window.innerHeight;
});
#endif

Renderer::Renderer(Game* game)
	: mOwner(game), mWindow(nullptr), mGL(nullptr), mFramebuffer(nullptr), mWidth(0), mHeight(0),
	  mCamera(nullptr) {
	mGL = std::make_unique<GLManager>();

	mWindow = SDL_CreateWindow("OpenGL", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with "
				  "your system/SDL installation");

		throw std::runtime_error("game.cpp: Failed to create SDL window");
	}
	SDL_SetWindowMinimumSize(mWindow, 480, 320);

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

#ifdef __EMSCRIPTEN__
	mHeight = browserHeight();
	mWidth = browserWidth();
	canvasResize();
	SDL_SetWindowSize(mWindow, mWidth, mHeight);
#else
	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);
#endif

	glViewport(0, 0, mWidth, mHeight);

#ifdef IMGUI
	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
	ImGui_ImplOpenGL3_Init("#version 400");
#endif

	mGL->printInfo();

	mFramebuffer = std::make_unique<Framebuffer>(mOwner);

	// Matrix uniform
	// mMatricesUBO = std::make_unique<UBO>(2 * sizeof(Eigen::Affine3f));
	// mMatricesUBO->bind(0);
}

Renderer::~Renderer() { SDL_DestroyWindow(mWindow); /* Other stuff are smart pointers */ }

void Renderer::setDemensions(int width, int height) {
	mWidth = width;
	mHeight = height;

	glViewport(0, 0, width, height);
	mFramebuffer->setDemensions(width, height);
}

void Renderer::draw() const {
#ifdef IMGUI
	ImGui::Render();
#endif

	glClearColor(0.1f, 0.5f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// mMatricesUBO->bind(0);
	// mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), mCamera->getProjectionMatrix());
	// mMatricesUBO->set(1 * sizeof(Eigen::Affine3f), mCamera->getViewMatrix());

	for (const auto& sprite : mDrawables) {
		// TODO: Depth?
		Shader* const shader = sprite->getShader();
		shader->activate();
		shader->set("viewPos", mCamera->getOwner()->getPosition()); // Bruh how come I forgot
		shader->set("view", mCamera->getViewMatrix());
		shader->set("proj", mCamera->getProjectionMatrix());
		// shader->bind("Matrices", 0);

		setLights(shader);

		sprite->draw();
	}

	mFramebuffer->swap(mWindow);
}

void Renderer::reload() const {
	for (const auto& sprite : mDrawables) {
		sprite->reload();
	}
}

void Renderer::setCamera(class CameraComponent* camera) {
	mCamera = camera;
	// mMatricesUBO->set(0, camera->getProjectionMatrix());
}

void Renderer::addSprite(DrawComponent* sprite) {
	// sprite->getShader()->bind("Matrices", 0);

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
	const Eigen::Vector4f lightPos(1.2f, 1.0f, 2.0f, 1.0f);

	shader->set("dirLight.direction", -0.2f, -1.0f, -0.3f);
	shader->set("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	shader->set("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	shader->set("dirLight.specular", 0.5f, 0.5f, 0.5f);

	// TODO: Point light

	shader->set("spotLight.position", mCamera->getOwner()->getPosition());
	shader->set("spotLight.direction", mCamera->getOwner()->getForward());
	shader->set("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	shader->set("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	shader->set("spotLight.specular", 1.0f, 1.0f, 1.0f);
	shader->set("spotLight.constant", 1.0f);
	shader->set("spotLight.linear", 0.09f);
	shader->set("spotLight.quadratic", 0.032f);
	shader->set("spotLight.cutOff", cos(toRadians(12.5f)));
	shader->set("spotLight.outerCutOff", cos(toRadians(15.0f)));
}

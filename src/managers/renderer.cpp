#include "managers/renderer.hpp"

#include "actors/actor.hpp"
#include "components/cameraComponent.hpp"
#include "game.hpp"
#include "managers/glManager.hpp"
#include "opengl/framebuffer.hpp"
#include "opengl/mesh.hpp"
#include "opengl/model.hpp"
#include "opengl/shader.hpp"
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
	  mWindowMesh(nullptr), mModel(nullptr) {
	mGL = std::make_unique<GLManager>();

	mWindow = SDL_CreateWindow("Golf", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

	mModel = std::make_unique<Model>(mOwner->fullPath("models" SEPARATOR "backpack.obj"), mOwner);

	const std::vector<Vertex> vertices = {
		{{-0.5f, +0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Top left
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bot left
		{{+0.5f, +0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // Top right
		{{+0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Bot right
	};
	const std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2};
	const std::vector<std::pair<Texture*, TextureType>> textures = {
		std::make_pair(mOwner->getTexture("windows.png"), TextureType::DIFFUSE)};

	mWindowMesh = std::make_unique<Mesh>(vertices, indices, textures);

	mFramebuffer = std::make_unique<Framebuffer>(mOwner);
}

Renderer::~Renderer() { SDL_DestroyWindow(mWindow); }

void Renderer::setDemensions(int width, int height) {
	mWidth = width;
	mHeight = height;

	glViewport(0, 0, width, height);
	mFramebuffer->setDemensions(width, height);
}

void Renderer::draw(CameraComponent* camera) {
#ifdef IMGUI
	ImGui::Render();
#endif

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader* backpackShader = mOwner->getShader("basic.vert", "basic.frag");
	backpackShader->activate();

	backpackShader->set("view", camera->getViewMatrix());
	backpackShader->set("proj", camera->getProjectionMatrix());
	Eigen::Affine3f modelMat = Eigen::Affine3f::Identity();
	// modelMat.rotate(Eigen::AngleAxisf(toRadians(time), Eigen::Vector3f::UnitY()));
	backpackShader->set("model", modelMat);

	const Eigen::Vector4f lightPos(1.2f, 1.0f, 2.0f, 1.0f);

	backpackShader->set("dirLight.direction", -0.2f, -1.0f, -0.3f);
	backpackShader->set("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	backpackShader->set("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	backpackShader->set("dirLight.specular", 0.5f, 0.5f, 0.5f);

	backpackShader->set("spotLight.position", camera->getOwner()->getPosition());
	backpackShader->set("spotLight.direction", camera->getOwner()->getForward());
	backpackShader->set("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	backpackShader->set("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	backpackShader->set("spotLight.specular", 1.0f, 1.0f, 1.0f);
	backpackShader->set("spotLight.constant", 1.0f);
	backpackShader->set("spotLight.linear", 0.09f);
	backpackShader->set("spotLight.quadratic", 0.032f);
	backpackShader->set("spotLight.cutOff", cos(toRadians(12.5f)));
	backpackShader->set("spotLight.outerCutOff", cos(toRadians(15.0f)));

	mModel->draw(backpackShader);

	Shader* windowShader = mOwner->getShader("basic.vert", "window.frag");
	windowShader->activate();

	windowShader->set("view", camera->getViewMatrix());
	windowShader->set("proj", camera->getProjectionMatrix());

	modelMat.setIdentity();
	modelMat.translate(Eigen::Vector3f(0.0f, 1.0f, 0.0f));
	modelMat.scale(2);
	windowShader->set("model", modelMat);

	mWindowMesh->draw(windowShader);

	modelMat.setIdentity();
	modelMat.translate(Eigen::Vector3f(0.2f, 0.0f, 0.4f));
	modelMat.translate(Eigen::Vector3f(0.0f, 1.0f, 0.0f));
	modelMat.scale(2);

	windowShader->set("model", modelMat);

	mWindowMesh->draw(windowShader);

	mFramebuffer->swap(mWindow);
}

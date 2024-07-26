#include "game.hpp"

#include "actors/actor.hpp"
#include "actors/player.hpp"
#include "components/cameraComponent.hpp"
#include "managers/glManager.hpp"
#include "managers/shaderManager.hpp"
#include "managers/textureManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/model.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "third_party/Eigen/src/Core/Matrix.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstdint>
#include <memory>
#include <string>
#include <third_party/Eigen/Dense>
#include <third_party/glad/glad.h>

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

Game::Game()
	: mWindow(nullptr), mGL(nullptr), mTextures(nullptr), mShaders(nullptr), mCamera(nullptr),
	  mModel(nullptr), mUpdatingActors(false), mWidth(0), mHeight(0), mTicks(0), mPaused(false) {
	ZoneScopedN("Setup context");

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

	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
	ImGui_ImplOpenGL3_Init("#version 400");

	SDL_Log("Finished Initializing ImGUI");
#endif

	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
	} else {
		mBasePath = std::string(".") + SEPARATOR;
	}

	mTextures = std::make_unique<TextureManager>(mBasePath);
	mShaders = std::make_unique<ShaderManager>(mBasePath);

	// TODO: Icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_backpackShaderroySurface(icon);
	*/

	mGL->printInfo();

	mTicks = SDL_GetTicks();

	SDL_SetRelativeMouseMode(1);

	setup();
}

void Game::setup() {
	ZoneScopedN("Settup assets");

	SDL_Log("Setting up game");

	new Player(this);
	mModel = new Model(fullPath("models" + SEPARATOR + "backpack.obj"), this);

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f},  {0.0f,  0.0f, 0.0f}, {0.0f, 0.0f}},
		{{+0.5f, -0.5f, 0.0f},  {0.0f,  0.0f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.0f},  {0.0f,  0.0f, 0.0f}, {1.0f, 0.0f}},
		{{+0.5f,  0.5f, 0.0f},  {0.0f,  0.0f, 0.0f}, {1.0f, 1.0f}},
	};
	const std::vector<unsigned int> indices = {1, 3, 0, 3, 2, 0};
	const std::vector<std::pair<Texture*, TextureType>> textures = {std::make_pair(mTextures->get("windows.png"), TextureType::DIFFUSE)};

	mWindowMesh = new Mesh(vertices, indices, textures);

	SDL_Log("Successfully initialized OpenGL and game\n");
}

#ifdef DEBUG
#include <filesystem>
#endif

int Game::iterate() {
	ZoneScopedN("Iteration");

	if (mPaused) {
		mTicks = SDL_GetTicks();

		// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SDL_SetRelativeMouseMode(0);
		// SDL_GL_SwapWindow(mWindow);

		return 0;
	}
#ifdef __EMSCRIPTEN__
	// Web hack
	SDL_SetWindowSize(mWindow, browserWidth(), browserHeight());
#endif

#ifdef DEBUG
	static std::filesystem::file_time_type last_time;

	if (std::filesystem::last_write_time(fullPath("shaders")) != last_time) {
		last_time = std::filesystem::last_write_time(fullPath("shaders"));

		mShaders->reload();
	}
#endif

	gui();
	input();
	update();
	draw();

	return 0;
}

void Game::input() {
	ZoneScopedN("Input");

	const uint8_t* keys = SDL_GetKeyboardState(nullptr);

	mUpdatingActors = true;
	for (auto& actor : mActors) {
		actor->input(keys);
	}
	mUpdatingActors = false;
}

void Game::update() {
	ZoneScopedN("Update");
	// Update the game
	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05) {
		delta = 0.05;
	}
	mTicks = SDL_GetTicks();

	// Update the Actors
	mUpdatingActors = true;
	for (auto& actor : mActors) {
		actor->update(delta);
	}
	mUpdatingActors = false;

	// Append the pending actors
	std::copy(mPendingActors.begin(), mPendingActors.end(), std::back_inserter(mActors));
	mPendingActors.clear();

	// Remove the dead Actors
	std::vector<Actor*> deadActors;
	std::copy_if(mActors.begin(), mActors.end(), std::back_inserter(deadActors),
				 [](const Actor* actor) { return (actor->getState() == Actor::DEAD); });

	// Delete all the dead actors
	for (const auto& actor : deadActors) {
		delete actor;
	}
}

#ifdef IMGUI
void Game::gui() {
	ZoneScopedN("ImGui");

	static bool demoMenu = false;
	static bool vsync = true;
	static bool wireframe = false;

	// Update ImGui Frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", (1000.f / io.Framerate), io.Framerate);
		ImGui::Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices,
					io.MetricsRenderIndices, io.MetricsRenderIndices / 3);
		auto pos = mActors[0]->getPosition();
		ImGui::Text("%dx%dx%d", static_cast<int>(pos.x()), static_cast<int>(pos.y()),
					static_cast<int>(pos.z()));

		// ImGui::Checkbox("Debug", &debugMenu);
		ImGui::Checkbox("Demo", &demoMenu);
		ImGui::Checkbox("VSync", &vsync);
		ImGui::Checkbox("Wireframe", &wireframe);

		ImGui::End();
	}

	if (demoMenu) {
		ImGui::ShowDemoWindow(&demoMenu);
	}

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	SDL_GL_SetSwapInterval(static_cast<int>(vsync));
}
#else
void Game::gui() { ZoneScopedN("ImGui"); }
#endif

void Game::draw() {
	ZoneScopedN("Draw");
	TracyGpuZone("Draw");
#ifdef IMGUI
	ImGui::Render();
#endif

	// float time = static_cast<float>(SDL_GetTicks()) / 10;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader* backpackShader = mShaders->get("basic.vert", "basic.frag");
	backpackShader->activate();

	backpackShader->set("view", mCamera->getViewMatrix());
	backpackShader->set("proj", mCamera->getProjectionMatrix());
	Eigen::Affine3f modelMat = Eigen::Affine3f::Identity();
	// modelMat.rotate(Eigen::AngleAxisf(toRadians(time), Eigen::Vector3f::UnitY()));
	backpackShader->set("model", modelMat);

	const Eigen::Vector4f lightPos(1.2f, 1.0f, 2.0f, 1.0f);

	backpackShader->set("dirLight.direction", -0.2f, -1.0f, -0.3f);
	backpackShader->set("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	backpackShader->set("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	backpackShader->set("dirLight.specular", 0.5f, 0.5f, 0.5f);

	backpackShader->set("spotLight.position", mCamera->getOwner()->getPosition());
	backpackShader->set("spotLight.direction", mCamera->getOwner()->getForward());
	backpackShader->set("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	backpackShader->set("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	backpackShader->set("spotLight.specular", 1.0f, 1.0f, 1.0f);
	backpackShader->set("spotLight.constant", 1.0f);
	backpackShader->set("spotLight.linear", 0.09f);
	backpackShader->set("spotLight.quadratic", 0.032f);
	backpackShader->set("spotLight.cutOff", cos(toRadians(12.5f)));
	backpackShader->set("spotLight.outerCutOff", cos(toRadians(15.0f)));

	mModel->draw(backpackShader);

	Shader* windowShader = mShaders->get("basic.vert", "window.frag");
	windowShader->activate();

	windowShader->set("view", mCamera->getViewMatrix());
	windowShader->set("proj", mCamera->getProjectionMatrix());
	modelMat.translate(Eigen::Vector3f(0.0f, 1.0f, 0.0f));
	modelMat.scale(2);
	// modelMat.rotate(Eigen::AngleAxisf(toRadians(time), Eigen::Vector3f::UnitY()));
	windowShader->set("model", modelMat);

	mWindowMesh->draw(windowShader);

#ifdef IMGUI
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

	SDL_GL_SwapWindow(mWindow);
	TracyGpuCollect;
}

int Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);
#endif

	static bool rel = true;

	switch (event.type) {
		case SDL_EVENT_QUIT: {
			return 1;
		}

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
			if (event.window.windowID == SDL_GetWindowID(mWindow)) {
				return 1;
			}

			break;
		}

		case SDL_EVENT_KEY_DOWN: {
			if (event.key.key == SDLK_ESCAPE) {
				return 1;
			}
			if (event.key.key == SDLK_F1) {
				rel = !rel;
				SDL_SetRelativeMouseMode(static_cast<int>(rel));
			}
			if (event.key.key == SDLK_F2) {
#ifdef DEBUG
				if (mPaused) {
					mPaused = false;
				}
#endif
				mTextures->reload();
				mShaders->reload();
			}
			if (event.key.key == SDLK_F3) {
				mPaused = !mPaused;
			}
			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			mWidth = event.window.data1;
			mHeight = event.window.data2;
			glViewport(0, 0, event.window.data1, event.window.data2);

			break;
		}
	}

	return 0;
}

void Game::addActor(Actor* actor) {
	if (!mUpdatingActors) {
		mActors.emplace_back(actor);
	} else {
		mPendingActors.emplace_back(actor);
	}
}

void Game::removeActor(Actor* actor) {
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end()) {
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end()) {
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}

Texture* Game::getTexture(const std::string& name) { return mTextures->get(name); }

Game::~Game() {
	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	while (!mActors.empty()) {
		delete mActors.back();
	}
	while (!mPendingActors.empty()) {
		delete mPendingActors.back();
	}

	delete mModel;

	SDL_DestroyWindow(mWindow);
}

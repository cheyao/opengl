#include "game.hpp"

#include "actors/actor.hpp"
#include "actors/player.hpp"
#include "components/cameraComponent.hpp"
#include "managers/glManager.hpp"
#include "managers/shaderManager.hpp"
#include "managers/textureManager.hpp"
#include "opengl/model.hpp"
#include "opengl/shader.hpp"
#include "third_party/Eigen/src/Core/Matrix.h"
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
	: mWindow(SDL_CreateWindow("Golf", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)),
	  mGL(nullptr), mTextures(nullptr), mShaders(nullptr), mCamera(nullptr), mModel(nullptr),
	  mUpdatingActors(false), mWidth(0), mHeight(0), mTicks(0), mPaused(false) {
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with "
				  "your SDL installation");

		throw 1;
	}
	SDL_SetWindowMinimumSize(mWindow, 480, 320);

	mGL = std::make_unique<GLManager>(mWindow);
	mGL->printInfo();

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

	// TODO: Get a icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_DestroySurface(icon);
	*/

	mTicks = SDL_GetTicks();

	SDL_SetRelativeMouseMode(true);

	setup();
}

void Game::setup() {
	SDL_Log("Setting up game");

	new Player(this);
	mModel = new Model(fullPath("models" + SEPARATOR + "backpack.obj"), this);

	SDL_Log("Successfully initialized OpenGL and game\n");
}

#ifdef DEBUG
#include <filesystem>
#endif

int Game::iterate() {
	if (mPaused) {
		mTicks = SDL_GetTicks();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SDL_SetRelativeMouseMode(0);
		SDL_GL_SwapWindow(mWindow);

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
	const uint8_t* keys = SDL_GetKeyboardState(nullptr);

	mUpdatingActors = true;
	for (auto& actor : mActors) {
		actor->input(keys);
	}
	mUpdatingActors = false;
}

void Game::update() {
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
void Game::gui() {}
#endif

void Game::draw() {
#ifdef IMGUI
	ImGui::Render();
#endif

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader* dest = mShaders->get("basic.vert", "basic.frag");
	dest->activate();

	dest->set("view", mCamera->getViewMatrix());
	dest->set("proj", mCamera->getProjectionMatrix());
	Eigen::Affine3f modelMat = Eigen::Affine3f::Identity();
	dest->set("model", modelMat);

	// Eigen::Vector3f(1.0f, 0.3f, 0.5f).normalized();

	/*float time = static_cast<float>(SDL_GetTicks()) /
	 * 1000;*/

	Eigen::Vector4f lightPos(1.2f, 1.0f, 2.0f, 1.0f);

	dest->set("dirLight.direction", -0.2f, -1.0f, -0.3f);
	dest->set("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	dest->set("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
	dest->set("dirLight.specular", 0.5f, 0.5f, 0.5f);

	/*
	const Eigen::Vector3f pointLightPositions[] = {
		Eigen::Vector3f(0.7f, 0.2f, 2.0f),
	Eigen::Vector3f(2.3f, -3.3f, -4.0f),
		Eigen::Vector3f(-4.0f, 2.0f, -12.0f),
	Eigen::Vector3f(0.0f, 0.0f, -3.0f)};

	for (size_t i = 0; i < sizeof(pointLightPositions) /
	sizeof(pointLightPositions[0]); i++) { std::string light =
	"pointLights[0]."; light[12] += i;

		dest->set(light + "position", pointLightPositions[i]);
		dest->set(light + "ambient", 0.05f, 0.05f, 0.05f);
		dest->set(light + "diffuse", 0.8f, 0.8f, 0.8f);
		dest->set(light + "specular", 1.0f, 1.0f, 1.0f);
		dest->set(light + "constant", 1.0f);
		dest->set(light + "linear", 0.09f);
		dest->set(light + "quadratic", 0.032f);
	}
	*/

	dest->set("spotLight.position", mCamera->getOwner()->getPosition());
	dest->set("spotLight.direction", mCamera->getOwner()->getForward());
	dest->set("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	dest->set("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	dest->set("spotLight.specular", 1.0f, 1.0f, 1.0f);
	dest->set("spotLight.constant", 1.0f);
	dest->set("spotLight.linear", 0.09f);
	dest->set("spotLight.quadratic", 0.032f);
	dest->set("spotLight.cutOff", cos(toRadians(12.5f)));
	dest->set("spotLight.outerCutOff", cos(toRadians(15.0f)));

	mModel->draw(dest);

	/*
	source->activate();

	source->set("view", mCamera->mViewMatrix);
	source->set("proj", mCamera->mProjectionMatrix);

	source->set("aColor", 1.0f, 1.0f, 1.0f);

	for (const auto& pos : pointLightPositions) {
		modelMat.setIdentity();
		modelMat.translate(pos);
		modelMat.scale(0.2f);

		source->set("model", modelMat);

		glDrawElements(GL_TRIANGLES, mVertex->indices(),
	GL_UNSIGNED_INT, 0);
	}
	*/

#ifdef IMGUI
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

	SDL_GL_SwapWindow(mWindow);
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

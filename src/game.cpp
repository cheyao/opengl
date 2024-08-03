#include "game.hpp"

#include "actors/actor.hpp"
#include "actors/player.hpp"
#include "actors/world.hpp"
#include "managers/shaderManager.hpp"
#include "managers/textureManager.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/src/Core/Matrix.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <third_party/Eigen/Dense>
#include <third_party/glad/glad.h>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

Game::Game()
	: mTextures(nullptr), mShaders(nullptr), mRenderer(nullptr), mUpdatingActors(false), mTicks(0),
	  mBasePath(""), mPaused(false) {
	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
	} else {
		mBasePath = std::string(".") + SEPARATOR;
	}

	mTextures = std::make_unique<TextureManager>(mBasePath);
	mShaders = std::make_unique<ShaderManager>(mBasePath);

	mRenderer = new Renderer(this);

	// TODO: Icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_backpackShaderroySurface(icon);
	*/

	SDL_SetRelativeMouseMode(1);

	setup();
}

#ifdef DEBUG
#include <filesystem>
#endif

void Game::setup() {
	SDL_Log("Setting up game");

	new World(this);
	new Player(this);

	mTicks = SDL_GetTicks();

#ifdef DEBUG
	last_time = std::filesystem::last_write_time(fullPath("shaders"));
#endif

	SDL_Log("Successfully initialized OpenGL and game\n");
}

int Game::iterate() {
	if (mPaused) {
		mTicks = SDL_GetTicks();

		// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SDL_SetRelativeMouseMode(0);
		// Delay some time to not use too much CPU
		SDL_Delay(64);
		// SDL_GL_SwapWindow(mWindow);

		return 0;
	}
#ifdef __EMSCRIPTEN__
	// Web hack
	SDL_SetWindowSize(mWindow, browserWidth(), browserHeight());
#endif

#ifdef DEBUG
	if (std::filesystem::last_write_time(fullPath("shaders")) != last_time) {
		last_time = std::filesystem::last_write_time(fullPath("shaders"));

		mShaders->reload();
	}
#endif

	gui();
	input();
	update();
	draw();

#ifdef DEBUG
	// TODO:
	/*
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_Log("GLError: Invalid enum");
				break;
			case GL_INVALID_VALUE:
				SDL_Log("GLError: Invalid value");
				break;
			case GL_INVALID_OPERATION:
				SDL_Log("GLError: Invalid operation");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_Log("GLError: Invalid framebuffer op");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_Log("GLError: Out of memory");
				break;
		}
	}
	*/
#endif

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

void Game::gui() {
#ifdef IMGUI
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

		Player* p = nullptr;
		for (const auto& actor : mActors) {
			if (dynamic_cast<Player*>(actor) != nullptr) {
				p = dynamic_cast<Player*>(actor);
				break;
			}
		}
		if (p == nullptr) {
			return;
		}

		auto pos = p->getPosition();
		ImGui::Text("Player position: %dx%dx%d", static_cast<int>(pos.x()),
					static_cast<int>(pos.y()), static_cast<int>(pos.z()));

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
#endif
}

void Game::draw() { mRenderer->draw(); }

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
			return 1;

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
				// mTextures->reload(true);
				// TODO: Maybe textures
				mShaders->reload(true);
				mRenderer->reload();
			}
			if (event.key.key == SDLK_F3) {
				mPaused = !mPaused;
			}
			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			mRenderer->setDemensions(event.window.data1, event.window.data2);

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
Shader* Game::getShader(const std::string& vert, const std::string& frag) {
	return mShaders->get(vert, frag);
}

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

	delete mRenderer;
}

[[nodiscard]] int Game::getWidth() const { return mRenderer->getWidth(); }
[[nodiscard]] int Game::getHeight() const { return mRenderer->getHeight(); }

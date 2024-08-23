#include "game.hpp"

#include "components.hpp"
#include "managers/eventManager.hpp"
#include "managers/localeManager.hpp"
#include "managers/systemManager.hpp"
#include "scene.hpp"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <string>
#include <third_party/Eigen/Core>

#ifdef IMGUI
#ifdef GLES
#define IMGUI_IMPL_OPENGL_ES3
#endif
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

Game::Game() : mUIScale(1.0f), mTicks(0), mBasePath(""), mPaused(false), mQuit(false) {
	mUIScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	if (mUIScale == 0.0f) {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get display context scale: %s\033[0m",
			     SDL_GetError());

		mUIScale = 1.0f;
	}

	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
	} else {
		mBasePath = "";

		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get base path: %s\033[0m", SDL_GetError());
	}

	mEventManager = std::make_unique<EventManager>(this);
	mSystemManager = std::make_unique<SystemManager>(this);

	mLocaleManager = new LocaleManager(mBasePath);

	// TODO: Icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_backpackShaderroySurface(icon);
	*/

	SDL_GL_SetSwapInterval(1);

	setup();

	mScene = new Scene();
	EntityID entity = mScene->newEntity();
	mScene->emplace<Components::position>(entity, Eigen::Vector2f(0.0f, 0.0f));
	EntityID entity1 = mScene->newEntity();
	mScene->emplace<Components::velocity>(entity1, Eigen::Vector2f(0.0f, 0.0f));
	EntityID entity2 = mScene->newEntity();
	mScene->emplace<Components::position>(entity2, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::velocity>(entity2, Eigen::Vector2f(0.0f, 0.0f));
	EntityID entity3 = mScene->newEntity();
	mScene->emplace<Components::position>(entity3, Eigen::Vector2f(0.0f, 0.0f));
}

Game::~Game() {
	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	delete mLocaleManager;
}

void Game::setup() {
	SDL_Log("Setting up UIs");

	/*
	new ControlUI(this);
	new MainUI(this);
	*/

	SDL_Log("Successfully initialized OpenGL and UI\n");

	SDL_Log("Setting up game");

	SDL_Log("Successfully initialized Game World");

	mTicks = SDL_GetTicks();
}

SDL_AppResult Game::iterate() {
	if (mQuit) {
		return SDL_APP_SUCCESS;
	}

	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05f) {
		delta = 0.05f;

		SDL_Log("Delta > 0.5f, cutting frame short");
	}
	mTicks = SDL_GetTicks();

	gui();
	input();
	mSystemManager->update(mScene, delta);

#ifdef DEBUG
	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Out of memory\033[0m");
				break;
		}
	}
#endif

	return SDL_APP_CONTINUE;
}

void Game::gui() {
#ifdef IMGUI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	static bool wireframe = false;
	static bool vsync = true;
	static bool demo = false;

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("%.3f ms %.1f FPS", (1000.f / io.Framerate), io.Framerate);

		const char* locales[] = {"en", "fr", "zh-CN", "es"};
		static int current = 0;
		if (ImGui::Combo("language", &current, locales, IM_ARRAYSIZE(locales))) {
			mLocaleManager->changeLocale(locales[current]);
		}

		if (ImGui::Checkbox("VSync", &vsync)) {
			SDL_GL_SetSwapInterval(static_cast<int>(vsync));
		}

		if (glPolygonMode != nullptr) {
			// GLES doesn't have polygon mode
			if (ImGui::Checkbox("Wireframe", &wireframe)) {
				glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
			}
		}

		ImGui::Checkbox("Demo", &demo);

		ImGui::End();
	}

	if (demo) {
		ImGui::ShowDemoWindow(&demo);
	}
#endif
}

void Game::input() {
	/*
	for (const auto& ui : mUI) {
		ui->processInput(mEventManager->getKeystate());
	}
	*/
}

/*
void Game::update() {
	// Update the game
	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05f) {
		delta = 0.05f;

		SDL_Log("Delta > 0.5f, cutting frame short");
	}
	mTicks = SDL_GetTicks();

#ifdef IMGUI

	ImGui::Begin("UIs");
#endif
	for (const auto& ui : mUI) {
#ifdef IMGUI
		ImGui::Text("UI: %s %lx state: %d", ui->getName().data(), reinterpret_cast<uintptr_t>(ui),
			    ui->getState());
#endif

		if (ui->getState() == UIScreen::ACTIVE) {
			ui->update(delta);
		}
	}
#ifdef IMGUI
	ImGui::End();
#endif

	// Remove the dead UIs
	std::vector<const UIScreen*> deadUIs;
	std::copy_if(mUI.begin(), mUI.end(), std::back_inserter(deadUIs),
		     [](const auto* const ui) { return (ui->getState() == UIScreen::DEAD); });

	// Delete all the dead uis
	for (const auto& ui : deadUIs) {
		delete ui;
	}
}
*/

SDL_AppResult Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	/*
	switch (event.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_MOTION:
		case SDL_EVENT_MOUSE_WHEEL:
			if (io.WantCaptureMouse) {
				return SDL_APP_CONTINUE;
			}

			break;

		case SDL_EVENT_TEXT_INPUT:
			if (io.WantTextInput) {
				return SDL_APP_CONTINUE;
			}

			break;

		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
		case SDL_EVENT_GAMEPAD_ADDED:
		case SDL_EVENT_GAMEPAD_REMOVED:
			if (io.WantCaptureKeyboard) {
				return SDL_APP_CONTINUE;
			}

			break;

		default:
			break;
	}
	*/
#endif

	return mEventManager->manageEvent(event);
}

void Game::removeUI(UIScreen* ui) {
	const auto iter = std::find(mUI.begin(), mUI.end(), ui);
	[[likely]] if (iter != mUI.end()) {
		std::iter_swap(iter, mUI.end() - 1);
		mUI.pop_back();
	}
}

void Game::setKey(const size_t key, const bool val) { mEventManager->setKey(key, val); }

#include "game.hpp"

#include "components.hpp"
#include "managers/eventManager.hpp"
#include "managers/localeManager.hpp"
#include "managers/storageManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cstdint>
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
	if (mUIScale <= 0.0f) {
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

	mSystemManager = new SystemManager(this);
	mLocaleManager = new LocaleManager(mBasePath);

	mStorageManager = new StorageManager();

	// TODO: Icon
	/*
	SDL_SetWindowIcon(mWindow, icon);
	*/

	SDL_GL_SetSwapInterval(1);

	setup();
}

Game::~Game() {
	// Save state
	delete mStorageManager;

	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	delete mLocaleManager;
	delete mSystemManager;
	delete mScene;
}

void Game::setup() {
	SDL_Log("Setting up game");

	mScene = new Scene();
	EntityID player = mScene->newEntity();
	mScene->emplace<Components::texture>(player, mSystemManager->getTexture("stone.png", true));
	mScene->emplace<Components::position>(player, Eigen::Vector2f(400.0f, 400.0f));
	mScene->emplace<Components::velocity>(player, Eigen::Vector2f(0.0f, 0.0f));
	mScene->emplace<Components::input>(player, [](class Scene* scene, EntityID entity, const bool* scancodes,
						      [[maybe_unused]] const float delta) {
		Eigen::Vector2f& vel = scene->get<Components::velocity>(entity).vel;

		if (scancodes[SDL_SCANCODE_RIGHT] == true && vel.x() < 220) {
			vel.x() += 70;
		}

		if (scancodes[SDL_SCANCODE_LEFT] == true && vel.x() > -220) {
			vel.x() -= 70;
		}
	});
	mScene->emplace<Components::collision>(
		player, Eigen::Vector2f(0.0f, 0.0f),
		Eigen::Vector2f(mSystemManager->getTexture("stone.png", true)->getWidth(),
				mSystemManager->getTexture("stone.png", true)->getHeight()));
	mScene->emplace<Components::misc>(player, Components::misc::JUMP);

	EntityID block2 = mScene->newEntity();
	mScene->emplace<Components::texture>(block2, mSystemManager->getTexture("stone.png"));
	mScene->emplace<Components::position>(block2, Eigen::Vector2f(400.0f, 10.0f));
	mScene->emplace<Components::collision>(
		block2, Eigen::Vector2f(0.0f, 0.0f),
		Eigen::Vector2f(mSystemManager->getTexture("stone.png", true)->getWidth(),
				mSystemManager->getTexture("stone.png", true)->getHeight()),
		true);

	EntityID text = mScene->newEntity();
	mScene->emplace<Components::text>(text, "controls");
	mScene->emplace<Components::position>(text, Eigen::Vector2f(10.0f, 10.0f));

	SDL_Log("Successfully initialized Game World");

	mTicks = SDL_GetTicks();
}

SDL_AppResult Game::iterate() {
	if (mQuit) {
		return SDL_APP_SUCCESS;
	}

	static uint64_t ticks = SDL_GetTicks();
	static std::uint64_t frames = 0;
	++frames;
	if (frames % 60 == 0) {
		SDL_Log("FPS: %f", static_cast<float>(frames) / ((static_cast<float>(SDL_GetTicks()) - ticks) / 1000));
		frames = 0;
		ticks = SDL_GetTicks();
	}

	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05f) {
		delta = 0.05f;

		SDL_Log("Delta > 0.5f, cutting frame short");
	}
	mTicks = SDL_GetTicks();

	gui();
	mSystemManager->update(mScene, delta);

#ifdef DEBUG
	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mGLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mGLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mGLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mGLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mGLError: Out of memory\033[0m");
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

		// GLES doesn't have polygon mode
		if (glPolygonMode != nullptr) {
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

SDL_AppResult Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);

	ImGuiIO& io = ImGui::GetIO();

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
#endif

	return mEventManager->manageEvent(event);
}

void Game::setKey(const size_t key, const bool val) { mEventManager->setKey(key, val); }
[[nodiscard]] bool* Game::getKeystate() { return mEventManager->getKeystate(); };

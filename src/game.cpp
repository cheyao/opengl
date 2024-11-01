#include "game.hpp"

#include "managers/eventManager.hpp"
#include "managers/localeManager.hpp"
#include "managers/storageManager.hpp"
#include "managers/systemManager.hpp"
#include "scenes/level.hpp"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cinttypes>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#ifdef DEBUG
#include <chrono>
#endif

Game::Game()
	: mEventManager(nullptr), mSystemManager(nullptr), mLocaleManager(nullptr), mStorageManager(nullptr),
	  mUIScale(1.0f), mTicks(0), mBasePath(""), mPaused(false), mQuit(false), mCurrentLevel(nullptr) {
#ifdef DEBUG
	const auto begin = std::chrono::high_resolution_clock::now();
#endif
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

		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\033[31mFailed to get base path: %s\033[0m", SDL_GetError());
	}

	// First initialize these subsystems because the other ones need it
	mEventManager = std::make_unique<EventManager>(this);

	mSystemManager = new SystemManager(this);
	mLocaleManager = new LocaleManager(mBasePath);

	// TODO: Icon
	// SDL_SetWindowIcon(mWindow, icon);

	mStorageManager = new StorageManager(this);

	try {
		mStorageManager->restore();
	} catch (const std::exception& error) {
		SDL_Log("Failed to read saved state with error %s, creating new state", error.what());

		mCurrentLevel = new Level(this);
		mCurrentLevel->create();
	}

	SDL_assert(mCurrentLevel != nullptr);

	mTicks = SDL_GetTicks();

#ifdef DEBUG
	const auto end = std::chrono::high_resolution_clock::now();
	std::stringstream time;
	time << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << "ns ("
	     << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "milis)";
	SDL_Log("Startup took %s", time.str().data());
#endif

#ifdef DEBUG
	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER,
					     "\x1B[31mInit GLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\x1B[31mInit GLError: Out of memory\033[0m");
				break;
		}
	}
#endif
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
	delete mCurrentLevel;
}

SDL_AppResult Game::iterate() {
#ifdef DEBUG
	const auto begin = std::chrono::high_resolution_clock::now();
#endif

	if (mQuit) {
		return SDL_APP_SUCCESS;
	}

	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05f) {
		delta = 0.05f;

		SDL_Log("\033[33mDelta > 0.5f, cutting frame short\033[0m");
	}
	mTicks = SDL_GetTicks();

	gui();
	mSystemManager->update(mCurrentLevel->getScene(), delta);

#ifdef DEBUG
	const auto end = std::chrono::high_resolution_clock::now();
	std::stringstream time;
	static std::chrono::nanoseconds::rep framerate = 0;
	static std::uint64_t count = 0;
	framerate += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
	count++;
	if (count == 100) {
		SDL_Log("Average draw time (last 100 frames): %" PRIu64 "ns", static_cast<std::uint64_t>(framerate) / count);
		framerate = 0;
		count = 0;
	}

	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\333[31mGLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mGLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mGLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mGLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mGLError: Out of memory\033[0m");
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

// FIXME: There shouldn't be so much inderection
void Game::setKey(const std::size_t key, const bool val) { mEventManager->setKey(key, val); }
[[nodiscard]] bool* Game::getKeystate() { return mEventManager->getKeystate(); };

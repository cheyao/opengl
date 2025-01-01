#include "game.hpp"

#include "managers/eventManager.hpp"
#include "managers/localeManager.hpp"
#include "managers/storageManager.hpp"
#include "managers/systemManager.hpp"
#include "scene.hpp"
#include "scenes/level.hpp"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

Game::Game()
	: mEventManager(nullptr), mSystemManager(nullptr), mLocaleManager(nullptr), mCurrentLevel(nullptr),
	  mStorageManager(nullptr), mTicks(0), mBasePath("") {
	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
	} else {
		mBasePath = "";

		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\033[31mFailed to get base path: %s\033[0m", SDL_GetError());
	}
}

void Game::init() {
	const auto begin = std::chrono::high_resolution_clock::now();

	// First initialize these subsystems because the other ones need it
	mEventManager = std::make_unique<EventManager>();

	mSystemManager = std::make_unique<SystemManager>();
	mLocaleManager = std::make_unique<LocaleManager>(mBasePath);

	mCurrentLevel = std::make_unique<Level>();
	mStorageManager = std::make_unique<StorageManager>();

	if (mStorageManager->restore() != 0) {
		SDL_Log("\033[31mFailed to read saved state, creating new state\033[0m");

		mCurrentLevel->create();
	}

	mTicks = SDL_GetTicks();

	const auto end = std::chrono::high_resolution_clock::now();
	std::stringstream time;
	time << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << "ns ("
	     << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "milis)";
	SDL_Log("Startup took %s", time.str().data());

#ifdef DEBUG
	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mInit GLError: Invalid enum\033[0m");
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
	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif
}

SDL_AppResult Game::iterate() {
	const auto begin = std::chrono::high_resolution_clock::now();

	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.1f) {
		delta = 0.1f;

		SDL_Log("\033[33mDelta > 0.1f, cutting frame short\033[0m");
	}
	mTicks = SDL_GetTicks();

	mEventManager->update();

	gui();
	mCurrentLevel->update(delta);
	mSystemManager->update(mCurrentLevel->getScene(), delta);

	const auto end = std::chrono::high_resolution_clock::now();
	std::stringstream time;
	static std::chrono::nanoseconds::rep framerate = 0;
	static std::uint64_t count = 0;
	framerate += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
	count++;

	const auto uniqueEntity = [this]() {
		const auto tid = this->mCurrentLevel->getScene()->newEntity();
		this->mCurrentLevel->getScene()->emplace<Components::text>(tid, "!Average framerate: 100FPS");
		this->mCurrentLevel->getScene()->emplace<Components::position>(
			tid, Eigen::Vector2f(0, std::numeric_limits<float>::infinity()));
		return tid;
	};
	static EntityID tid = uniqueEntity();

	if (count == 100) {
		SDL_Log("Average draw time (last 100 frames): %" PRIu64 "ns",
			static_cast<std::uint64_t>(framerate) / count);

		mCurrentLevel->getScene()->get<Components::text>(tid).mID =
			"!Average framerate: " +
			std::to_string(static_cast<std::uint64_t>(
				10e8 / (static_cast<double>(static_cast<std::uint64_t>(framerate)) / count)));

		framerate = 0;
		count = 0;
	}

#ifdef DEBUG
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
		ImGui::Begin("Developer menu");

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("%.3f ms %.1f FPS", (1000.f / io.Framerate), io.Framerate);

		const char* locales[] = {"en", "fr", "zh-CN"};
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
#endif

	return mEventManager->manageEvent(event);
}

// FIXME: There shouldn't be so much inderection
void Game::setKey(const std::size_t key, const bool val) { mEventManager->setKey(key, val); }
[[nodiscard]] std::span<bool> Game::getKeystate() { return mEventManager->getKeystate(); };

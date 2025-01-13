#include "game.hpp"

#include "components/furnace.hpp"
#include "items.hpp"
#include "managers/eventManager.hpp"
#include "managers/localeManager.hpp"
#include "managers/storageManager.hpp"
#include "managers/systemManager.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "scenes/level.hpp"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_events.h>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

std::vector<std::pair<std::uint8_t*, std::uint32_t>> mAudio;

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>

void audioSucceeded(emscripten_fetch_t* fetch) {
	SDL_Log("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
	// The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
	SDL_AudioSpec spec;
	std::uint8_t* wav_data;
	std::uint32_t wav_data_len;

	if (SDL_LoadWAV_IO(SDL_IOFromConstMem(fetch->data, fetch->numBytes), true, &spec, &wav_data, &wav_data_len)) {
		mAudio.emplace_back(std::make_pair(wav_data, wav_data_len));
		SDL_Log("Queued wav with %d %d %d", spec.freq, spec.channels, spec.format);
	} else {
		SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
	}

	emscripten_fetch_close(fetch);
}

void audioFailed(emscripten_fetch_t* fetch) {
	SDL_Log("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
	emscripten_fetch_close(fetch); // Also free data on failure.
}
#endif

Game::Game()
	: mEventManager(nullptr), mSystemManager(nullptr), mLocaleManager(nullptr), mCurrentLevel(nullptr),
	  mStorageManager(nullptr), mTicks(0), mStream(nullptr) {}

void Game::init() {
	const auto begin = std::chrono::high_resolution_clock::now();

	// First initialize these subsystems because the other ones need it
	mEventManager = std::make_unique<EventManager>();

	mSystemManager = std::make_unique<SystemManager>();
	mLocaleManager = std::make_unique<LocaleManager>();

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

	// static auto sign = Inventory(Eigen::Vector2f(0, 0), "ui/sign.png");
	// mSystemManager->getUISystem()->addScreen(&sign);

	SDL_AudioSpec spec;
	spec.format = SDL_AUDIO_S16LE;
	spec.channels = 2;
	spec.freq = 48000;
#ifdef __EMSCRIPTEN__
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = audioSucceeded;
	attr.onerror = audioFailed;
#else
	std::uint8_t* wav_data;
	std::uint32_t wav_data_len;
#endif

	for (const auto& file : registers::BACKGROUND_SOUNDS) {
#ifdef __EMSCRIPTEN__
		emscripten_fetch(
			&attr,
			("https://rawcdn.githack.com/cheyao/opengl/refs/heads/main/assets/sounds/" + file).data());
#else
		if (SDL_LoadWAV((getBasePath() + "assets/sounds/" + file).data(), &spec, &wav_data, &wav_data_len)) {
			mAudio.emplace_back(std::make_pair(wav_data, wav_data_len));
		} else {
			SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
		}
#endif
	}

	mStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
	SDL_ResumeAudioStreamDevice(mStream);

#ifdef DEBUG
	GLenum err = 0;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mInit GLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mInit GLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mInit GLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER,
					     "\033[31mInit GLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_CATEGORY_RENDER, "\033[31mInit GLError: Out of memory\033[0m");
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

	for (const auto [data, _] : mAudio) {
		SDL_free(data);
	}

	SDL_Quit();
}

void Game::save() { mStorageManager->save(); }

SDL_AppResult Game::iterate() {
	static std::size_t audioPtr = 0;
	if (mStream && !mAudio.empty()) {
		if (SDL_GetAudioStreamAvailable(mStream) < static_cast<int>(mAudio[audioPtr].second)) {
			SDL_PutAudioStreamData(mStream, mAudio[audioPtr].first, mAudio[audioPtr].second);
			SDL_FlushAudioStream(mStream);

			++audioPtr;
			if (audioPtr == mAudio.size()) {
				audioPtr = 0;
			}

			SDL_Log("Replenished sound buffer");
		}
	}

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

	// Tick the furnace
	static_cast<class FurnaceInventory*>(registers::CLICKABLES.at(Components::Item::FURNACE)())
		->tick(mCurrentLevel->getScene(), delta);
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

	/* Main menu */ {
		ImGui::Begin("Developer menu");

		ImGui::Text("WASD + Space to move");
		ImGui::Text("E to take inventory");
		ImGui::Text("ESC to close menu");
		ImGui::Text("Right click to interract");

		const char* locales[] = {"en", "fr"};
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

		ImGui::End();
	}
#endif
}

SDL_AppResult Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);
#endif

	if (event.type == SDL_EVENT_LOCALE_CHANGED) {
		mLocaleManager.reset(new LocaleManager());

		return SDL_APP_CONTINUE;
	}

	return mEventManager->manageEvent(event);
}

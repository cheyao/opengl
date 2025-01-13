#include "game.hpp"
#include "utils.hpp"

#ifdef IMGUI
#include "imgui.h"
#endif

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <exception>
#include <stdexcept>

// The main class is in charge of sdl
SDL_AppResult SDL_AppInit(void** appstate, int, char**) {
	SDL_srand(0);

	SDL_Log("Initializing cyao engine v1.0\n");

	SDL_SetAppMetadata("Cyao", "1.0", "com.cyao.2d-minecraft");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, "2D Minecraft");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, "https://github.com/cheyao/2d-minecraft");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, "game");

	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	// SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0"); // Translated in-engine (See eventManager.cpp)
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "0");

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to init SDL: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize SDL, there is something wrong with your system");

		return SDL_APP_FAILURE;
	}

	SDL_assert(SDL_WasInit(SDL_INIT_VIDEO) && "Why wasn't video initalized?");

	SDL_Log("Current platform: %s", SDL_GetPlatform());
#ifdef __ANDROID__
	SDL_Log("The SDL version is %d", SDL_GetAndroidSDKVersion());
#endif

#ifdef __cpp_exceptions
	try {
		Game::getInstance()->init();
		*appstate = Game::getInstance();
	} catch (const std::runtime_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mMain.cpp: Critical runtime error: %s\033[0m\n",
				error.what());

		return SDL_APP_FAILURE;
	} catch (const std::exception& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mMain.cpp: Critical exception: %s\033[0m\n",
				error.what());

		return SDL_APP_FAILURE;
	} catch (...) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mMain.cpp: Uncaught exception\033[0m\n");

		return SDL_APP_FAILURE;
	}
#else
	Game::getInstance()->init();
	*appstate = Game::getInstance();
#endif

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	SDL_assert(appstate != nullptr);

#ifdef __cpp_exceptions
	try {
		return static_cast<Game*>(appstate)->event(*event);
	} catch (...) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Uncaught event error\n");

		return SDL_APP_FAILURE;
	}
#else
	return static_cast<Game*>(appstate)->event(*event);
#endif
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	SDL_assert(appstate != nullptr);

#ifdef __cpp_exceptions
	try {
		return static_cast<Game*>(appstate)->iterate();
	} catch (const std::runtime_error& error) {
		SDL_Log("Main.cpp: Uncaught runtime error: %s", error.what());
		ERROR_BOX(string_format("Runtime error %s", error.what()).data());
	} catch (const std::exception& error) {
		SDL_Log("Main.cpp: Uncaught exception: %s", error.what());
		ERROR_BOX(string_format("Exception %s", error.what()).data());
	} catch (...) {
		SDL_Log("Main.cpp: Uncaught exception of unknown type");
	}

#ifdef IMGUI
	ImGui::EndFrame();
#endif

	return SDL_APP_CONTINUE;
#else
	return static_cast<Game*>(appstate)->iterate();
#endif
}

void SDL_AppQuit(void*, const SDL_AppResult result) {
	Game::getInstance()->save();

	if (result == SDL_APP_SUCCESS) {
		SDL_Log("Cyao game engine terminated successfully");
	} else {
		SDL_Log("Cyao game engine terminated with a faliure");
	}
}

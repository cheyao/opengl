#include "game.hpp"
#include "utils.hpp"

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

	SDL_Log("Initializing cyao engine v3.0\n");

	SDL_SetAppMetadata("Cyao", "1.0", "com.cyao.opengl");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, "Cyao's opengl Game Engine");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, "https://github.com/cheyao/opengl");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, "game");

	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0"); // Translated in-engine (See eventManager.cpp)
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "0");

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to init SDL: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize SDL, there is something wrong with your system");
		return SDL_APP_FAILURE;
	}

	SDL_assert(SDL_WasInit(SDL_INIT_VIDEO) && "Why wasn't video initalized?");

	SDL_Log("Hello! I am on %s!", SDL_GetPlatform());
#ifdef __ANDROID__
	SDL_Log("I am on android %d.", SDL_GetAndroidSDKVersion());
#endif

	try {
		*appstate = new Game();
	} catch (const std::runtime_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Critical runtime error: %s\n", error.what());

		return SDL_APP_FAILURE;
	} catch (const std::exception& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Critical exception: %s\n", error.what());

		return SDL_APP_FAILURE;
	} catch (...) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Uncaught error\n");

		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	SDL_assert(appstate != nullptr);

	try {
		return static_cast<Game*>(appstate)->event(*event);
	} catch (...) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Uncaught event error\n");

		return SDL_APP_FAILURE;
	}
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	SDL_assert(appstate != nullptr);

	try {
		return static_cast<Game*>(appstate)->iterate();
	} catch (const std::runtime_error& error) {
		SDL_Log("Main.cpp: Uncaught runtime error: %s", error.what());

#ifdef DEBUG
		static_cast<Game*>(appstate)->setPause(true);
#endif

		return SDL_APP_CONTINUE;
	} catch (const std::exception& error) {
		SDL_Log("Main.cpp: Uncaught exception: %s", error.what());

#ifdef DEBUG
		static_cast<Game*>(appstate)->setPause(true);
#endif

		return SDL_APP_CONTINUE;
	} catch (...) {
		SDL_Log("Main.cpp: Uncaught exception of unknown type");

		return SDL_APP_CONTINUE;
	}
}

void SDL_AppQuit(void* appstate) {
	if (appstate != nullptr) {
		delete static_cast<Game*>(appstate);
	}

	SDL_Quit();
	
	SDL_Log("Cyao game engine quitted");
}

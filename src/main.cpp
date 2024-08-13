#include "game.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cstdlib>
#include <ctime>
#include <stdexcept>

int SDL_AppInit(void** appstate, [[maybe_unused]] int argc, [[maybe_unused]] char** argv) {

	// The main class is in charge of sdl
	std::srand(std::time(nullptr));

	SDL_SetAppMetadata("Cyao", "1.0", "com.cyao.opengl");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, "OpengGL Game Engine");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, "https://github.com/cheyao/opengl");
	SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, "game");

	SDL_Log("Initializing cyao engine v3.0?\n");

#ifdef ANDROID
	SDL_Log("Yay, android!");
#endif

	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0"); // Translated in-engine
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "0");
	/*
#ifdef GLES
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#else
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
	*/

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to init SDL: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize SDL, there is something wrong with your system");
		return 1;
	}

	try {
		*appstate = new Game();
	} catch (const std::runtime_error& error) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Critical error: %s\n", error.what());

		return 1;
	} catch (...) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Uncaught error\n");

		return 1;
	}

	return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
	try {
		return static_cast<Game*>(appstate)->event(*event);
	} catch (...) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Main.cpp: Uncaught event error\n");

		return 1;
	}
}

int SDL_AppIterate(void* appstate) {
	try {
		return static_cast<Game*>(appstate)->iterate();
	} catch (const std::runtime_error& error) {
		SDL_Log("Main.cpp: Uncaught exception: %s", error.what());

#ifdef DEBUG
		static_cast<Game*>(appstate)->setPause(true);
#endif

		return 0;
	}
}

void SDL_AppQuit(void* appstate) {
	delete static_cast<Game*>(appstate);

	SDL_Quit();
}

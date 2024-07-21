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

int SDL_AppInit(void** appstate, int argc, char** argv) {
	(void)argc;
	(void)argv;

	// The main class is in charge of sdl

	std::srand(std::time(nullptr));

	SDL_Log("Initializing game\n");

#ifdef X11
	SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#endif
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft");
#ifdef GLES
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles");
#else
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to init SDL: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize SDL, there is something wrong with your system");
		return 1;
	}

	try {
		*appstate = new Game();
	} catch (...) {
		return 1;
	}

	SDL_SetRelativeMouseMode(1);

	return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
	return static_cast<Game*>(appstate)->event(*event);
}

int SDL_AppIterate(void* appstate) {
	try {
		return static_cast<Game*>(appstate)->iterate();
	} catch (...) {
#ifdef DEBUG
		static_cast<Game*>(appstate)->pause();

		return 0;
#else
		return 1;
#endif
	}
}

void SDL_AppQuit(void* appstate) {
	delete static_cast<Game*>(appstate);

	SDL_Quit();
}

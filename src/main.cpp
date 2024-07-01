#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cstdlib>
#include <ctime>

#include "game.hpp"

int SDL_AppInit(void **appstate, int argc, char **argv) {
	(void)argc;
	(void)argv;

	srand(time(nullptr));

	Game *game = new Game();
	if (game->init() != 0) {
		// If failed to init
		return 1;
	}
	*appstate = game;

	return 0;
}

int SDL_AppEvent(void *appstate, const SDL_Event *event) {
	return static_cast<Game *>(appstate)->event(*event);
}

int SDL_AppIterate(void *appstate) { 
	return static_cast<Game *>(appstate)->iterate(); 
}

void SDL_AppQuit(void *appstate) {
	delete static_cast<Game *>(appstate);
}


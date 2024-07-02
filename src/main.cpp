#include "common.hpp"
#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <cstdlib>
#include <ctime>

#include "game.hpp"

int SDL_AppInit(void **appstate, int argc, char **argv) {
	(void)argc;
	(void)argv;

	// The main class is in charge of sdl

	std::srand(std::time(nullptr));

	SDL_Log("Initializing game\n");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to init SDL: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize SDL, there is something wrong with your system");
		return 1;
	}

	Game *game = new Game();

	if (game->init() != 0) {
		return 1;
	}

	*appstate = game;

	return 0;
}

int SDL_AppEvent(void *appstate, const SDL_Event *event) {
	return static_cast<Game *>(appstate)->event(*event);
}

int SDL_AppIterate(void *appstate) { return static_cast<Game *>(appstate)->iterate(); }

void SDL_AppQuit(void *appstate) {
	delete static_cast<Game *>(appstate);

	SDL_Quit();
}

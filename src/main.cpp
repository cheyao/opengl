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
#include <string>
#include <system_error>

int SDL_AppInit(void** appstate, int argc, char** argv) {
	if (argc != 2) {
		SDL_Log("Usage: ./pano [file]");
		return 1;
	}

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
		*appstate = new Game(argv[1]);
	} catch (std::runtime_error e) {
		SDL_Log("Error: %s", e.what());
	} catch (...) {
		SDL_Log("Uncaught exception");
		return 1;
	}

	SDL_SetRelativeMouseMode(1);

	return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
	return static_cast<Game*>(appstate)->event(*event);
}

#ifdef DEBUG
#include <filesystem>
#endif

int SDL_AppIterate(void* appstate) {
	try {
		return static_cast<Game*>(appstate)->iterate();
	}
#ifdef DEBUG
	catch (const std::filesystem::filesystem_error& error) {

		SDL_Log("Filesystem Error:");
		SDL_Log("what():  %s", error.what());
		SDL_Log("path1(): %s", error.path1().c_str());
		SDL_Log("path2(): %s", error.path2().c_str());
		SDL_Log("code().value():    %d", error.code().value());
		SDL_Log("code().message():  %s", error.code().message().data());
		SDL_Log("code().category(): %s", error.code().category().name());

		// static_cast<Game*>(appstate)->pause();

		return 0;
	}
#endif
	catch (const std::runtime_error& error) {
#ifdef DEBUG
		SDL_Log("Uncaught exception: %s", error.what());

		static_cast<Game*>(appstate)->pause();

		return 0;
#else
		ERROR_BOX("Exception thrown, the game might not function correctly");

		return 0;
#endif
	}
}

void SDL_AppQuit(void* appstate) {
	delete static_cast<Game*>(appstate);

	SDL_Quit();
}

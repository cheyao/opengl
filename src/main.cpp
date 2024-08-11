#include "game.hpp"
#include "opengl/renderer.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
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

	SDL_Log("Initializing game\n");

#ifdef ANDROID
	SDL_Log("Yay, android!");
#endif

	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
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

	static_cast<Game*>(*appstate)->getRenderer()->setWindowRelativeMouseMode(1);

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

#ifdef HOT
#include <filesystem>
#endif

int SDL_AppIterate(void* appstate) {
	try {
		return static_cast<Game*>(appstate)->iterate();
	}
#ifdef HOT
	catch (const std::filesystem::filesystem_error& error) {

		SDL_Log("Main.cpp: Filesystem Error:");
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
		SDL_Log("Main.cpp: Uncaught exception: %s", error.what());

		static_cast<Game*>(appstate)->pause();
#else
		ERROR_BOX("Main.cpp: Exception thrown, the game might not continue to function correctly");
#endif

		return 0;
	}
}

void SDL_AppQuit(void* appstate) {
	delete static_cast<Game*>(appstate);

	SDL_Quit();
}

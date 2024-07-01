#include "game.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <string>
#include <utility>

#include "ball.hpp"
#include "common.hpp"

#ifdef IMGUI
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <imgui.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

// Hacks to get browser width and hight
EM_JS(int, browserHeight, (), { return window.innerHeight; });
EM_JS(int, browserWidth, (), { return window.innerWidth; });
#endif

#include "actor.hpp"
#include "spriteComponent.hpp"

#define TEXTURES_TO_LOAD "assets/Ball.png", "assets/Hole.png"

Game::Game()
    : mWindow(nullptr),
      mRenderer(nullptr),
      mUpdatingActors(false),
      mWindowWidth(1024),
      mWindowHeight(768),
      mMouse(),
      mBasePath(""),
      mRedraw(true) {}

int Game::init() {
	std::srand(std::time(nullptr) *
		   2);	// People always use srand(time()); so why not *2 for
			// more randomness?
	SDL_Log("Initializing game\n");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_TIMER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "Failed to init SDL: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR!",
					 "Failed to initialize SDL, there is "
					 "something wrong with your system",
					 nullptr);
		return 1;
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "Failed to init SDL_image: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(
		    SDL_MESSAGEBOX_ERROR, "ERROR!",
		    "Failed to initialize SDL_image, there is "
		    "something wrong with your system",
		    nullptr);
		return 1;
	}

	// This hint is for ImGUI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
	SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "2");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");

	// Create window and Renderer
#ifdef __EMSCRIPTEN__
	// Hacks for EMSCRIPTEN Full screen
	mWindowWidth = browserWidth();
	mWindowHeight = browserHeight();

	SDL_Log("The web window is %dx%d", mWindowWidth, mWindowHeight);
#endif
	mWindow = SDL_CreateWindow("Golf", mWindowWidth, mWindowHeight,
				   SDL_WINDOW_RESIZABLE);
	if (mWindow == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "Failed to create window: %s\n", SDL_GetError());
		return 1;
	}

	mRenderer = SDL_CreateRenderer(mWindow, nullptr);
	if (mRenderer == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "Failed to create renderer: %s", SDL_GetError());
		return 1;
	}
	SDL_SetRenderVSync(mRenderer, 1);

	// Set to 1024*768
	SDL_SetRenderLogicalPresentation(mRenderer, 1024, 768,
					 SDL_LOGICAL_PRESENTATION_LETTERBOX,
					 SDL_SCALEMODE_BEST);
#ifdef IMGUI
	// Init ImGUI
	SDL_Log("Initializing ImGUI");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef __EMSCRIPTEN__
	io.IniFilename = nullptr;
#endif

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLRenderer(mWindow, mRenderer);
	ImGui_ImplSDLRenderer3_Init(mRenderer);

	SDL_Log("Finished Initializing ImGUI");
#endif

	mTicks = SDL_GetTicks();

	char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath += basepath;
		SDL_free(basepath);  // We gotta free da pointer UwU
	}

	const std::vector<std::string> texturesToLoad{TEXTURES_TO_LOAD};
	for (const auto& texture : texturesToLoad) {
		int response = loadTexture(texture);

		if (response != 0) {
			return response;
		}
	}

	// Reset mouse state
	mMouse.captured = false;

	new Ball(this);

	SDL_Log("Successfully initialized game\n");
	return 0;
}

void Game::input() {
	mUpdatingActors = true;
	for (auto& actor : mActors) {
		actor->input();
	}
	mUpdatingActors = false;
}

void Game::update() {
	// Hack for web window resizing
#ifdef __EMSCRIPTEN__
	if (browserWidth() != mWindowWidth ||
	    browserHeight() != mWindowHeight) {
		mWindowWidth = browserWidth();
		mWindowHeight = browserHeight();

		SDL_SetWindowSize(mWindow, mWindowWidth, mWindowHeight);
	}
#endif

	// Update cursor position
	SDL_GetMouseState(&mMouse.position.x, &mMouse.position.y);
	SDL_RenderCoordinatesFromWindow(mRenderer, mMouse.position.x,
					mMouse.position.y, &mMouse.position.x,
					&mMouse.position.y);

	// Update the game
	float delta = (SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05) {
		delta = 0.05;
	}
	mTicks = SDL_GetTicks();

	// Update the Actors
	mUpdatingActors = true;
	for (auto& actor : mActors) {
		actor->update(delta);
	}
	mUpdatingActors = false;  

	// If no ball, add a ball
	if (mActors.empty()) {
		new Ball(this);

		redraw();
	}

	// Append the pending actors
	std::copy(mPendingActors.begin(), mPendingActors.end(),
		  std::back_inserter(mActors));
	mPendingActors.clear();

	// Remove the dead Actors
	std::vector<Actor*> deadActors;
	std::copy_if(mActors.begin(), mActors.end(),
		     std::back_inserter(deadActors), [](const Actor* actor) {
			     return (actor->getState() == Actor::STATE_DEAD);
		     });

	// Delete all the dead actors
	for (auto& actor : deadActors) {
		delete actor;
	}
}

#ifdef IMGUI
// ImGUI private vars: Bad code but why care when this is for debugging?
bool statisticsMenu = false;
bool debugMenu = false;
bool demoMenu = false;

float mScale = 1.f;
float x = 100, y = 100;
bool vsync = true;
#endif

void Game::gui() {
#ifdef IMGUI
	// Update ImGui Frame
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGui::Checkbox("Statistics", &statisticsMenu);
		ImGui::Checkbox("Debug", &debugMenu);
		ImGui::Checkbox("Demo", &demoMenu);

		ImGui::End();
	}

	if (demoMenu) {
		ImGui::ShowDemoWindow(&demoMenu);
	}

	if (statisticsMenu) {
		ImGui::Begin("Statistics", &statisticsMenu);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
			    (1000.f / io.Framerate), io.Framerate);
		ImGui::Text("There are %zu actors", mActors.size());
		ImGui::Text("There are %zu sprites", mSprites.size());

		ImGui::End();
	}

	if (debugMenu) {
		ImGui::Begin("Debug", &debugMenu);

		ImGui::Checkbox("VSync", &vsync);
		SDL_SetRenderVSync(mRenderer, vsync);

		ImGui::SliderFloat("Ball size", &mScale, 0, 10);
		ImGui::SliderFloat("Ball x", &x, 0, 1024);
		ImGui::SliderFloat("Ball y", &y, 0, 768);
		if (ImGui::Button("Set")) {
			for (auto& actor : mActors) {
				if (dynamic_cast<Ball*>(actor) != nullptr) {
					actor->setScale(mScale);
					actor->setPosition(Vector2(x, y));
				}
			}
		}

		ImGui::Text("Mouse: %d captured: %d", mMouse.type,
			    mMouse.captured);

		ImGui::End();
	}
#endif
}

void Game::draw() {
#ifdef IMGUI
	ImGui::Render();
#else
	// Optimize!
	if (!mRedraw) {
		return;
	}
	mRedraw = false;
#endif

	SDL_SetRenderDrawColor(mRenderer, 83, 252, 227, 255);
	SDL_RenderClear(mRenderer);
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

	for (auto& sprite : mSprites) {
		sprite->draw(mRenderer);
	}

#ifdef IMGUI
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), mRenderer);
#endif

	SDL_RenderPresent(mRenderer);
}

int Game::loadTexture(const std::string& textureName) {
	SDL_Texture* texture =
	    IMG_LoadTexture(mRenderer, (mBasePath + textureName).c_str());
	if (texture == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "Failed to load texture: %s", IMG_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR!",
					 "Failed to load one texture.",
					 mWindow);
		return 1;
	}

	mTextures[textureName] = texture;

	return 0;
}

int Game::iterate() {
	// Loop
	input();
	update();
	gui();
	draw();

	return 0;
}

int Game::event(const SDL_Event& constEvent) {
	SDL_Event event = constEvent;
	SDL_ConvertEventToRenderCoordinates(mRenderer, &event);

#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);
	const ImGuiIO& io = ImGui::GetIO();
#else
	// Voiding out all the expressions
	union {
		bool WantCaptureMouse = false;
		bool WantCaptureKeyboard;
	} io;
#endif

	switch (event.type) {
		case SDL_EVENT_QUIT: {
			return 1;
		}

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
			if (event.window.windowID == SDL_GetWindowID(mWindow)) {
				return 1;
			}

			break;
		}

		case SDL_EVENT_KEY_DOWN: {
			if (io.WantCaptureKeyboard) {
				break;
			}

			mKeyboard[event.key.keysym.sym] = true;
			break;
		}

		case SDL_EVENT_KEY_UP: {
			if (io.WantCaptureKeyboard) {
				break;
			}

			mKeyboard[event.key.keysym.sym] = false;
			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			mWindowWidth = event.window.data1;
			mWindowHeight = event.window.data2;
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			if (io.WantCaptureMouse) {
				break;
			}

			if (event.button.windowID != SDL_GetWindowID(mWindow)) {
				break;
			}

			mMouse.type =
			    static_cast<enum Mouse::type>(event.button.button);
			mMouse.position =
			    Vector2(event.button.x, event.button.y);
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_UP: {
			if (io.WantCaptureMouse) {
				break;
			}

			mMouse.type = Mouse::BUTTON_STATE_UP;
			break;
		}
	}

	return 0;
}

void Game::addActor(Actor* actor) {
	if (!mUpdatingActors) {
		mActors.emplace_back(actor);
	} else {
		mPendingActors.emplace_back(actor);
	}
}

void Game::removeActor(Actor* actor) {
	auto iter =
	    std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end()) {
		// Swap to end of vector
		// and pop off (avoid
		// erase copies)
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	// Is it in actors?
	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end()) {
		// Swap to end of vector
		// and pop off (avoid
		// erase copies)
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}

void Game::addSprite(SpriteComponent* sprite) {
	int drawOrder = sprite->getDrawOrder();
	auto iter = mSprites.begin();

	for (; iter != mSprites.end(); ++iter) {
		if (drawOrder < (*iter)->getDrawOrder()) {
			break;
		}
	}

	mSprites.insert(iter, sprite);
}

void Game::removeSprite(SpriteComponent* sprite) {
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);

	if (iter != mSprites.end()) {
		mSprites.erase(iter);
	}
}

void Game::addHole(class Hole* hole) { mHoles.emplace_back(hole); }

void Game::removeHole(class Hole* hole) {
	auto iter = std::find(mHoles.begin(), mHoles.end(), hole);

	if (iter != mHoles.end()) {
		mHoles.erase(iter);
	}
}

Game::~Game() {
	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	while (!mActors.empty()) {
		delete mActors.back();
	}

	while (!mPendingActors.empty()) {
		delete mActors.back();
	}

	for (auto& texture : mTextures) {
		if (texture.second != nullptr) {
			SDL_DestroyTexture(texture.second);
		}
	}

	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	IMG_Quit();
	SDL_Quit();
}


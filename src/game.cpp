#include "game.hpp"

#include "actors/actor.hpp"
#include "actors/cube.hpp"
#include "actors/player.hpp"
#include "actors/sun.hpp"
#include "actors/world.hpp"
#include "managers/eventManager.hpp"
#include "managers/shaderManager.hpp"
#include "managers/textureManager.hpp"
#include "opengl/renderer.hpp"
#include "ui/freetype.hpp"
#include "ui/mainMenu.hpp"
#include "utils.hpp"

#ifdef ANDROID
#include "ui/controlUI.hpp"
#endif

#include <SDL3/SDL.h>
#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <third_party/Eigen/Core>
#include <third_party/glad/glad.h>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

Game::Game()
	: mTextures(nullptr), mShaders(nullptr), mRenderer(nullptr), mFontManager(nullptr),
	  mActorMutex(SDL_CreateMutex()), mUIScale(1.0f), mTicks(0), mBasePath(""), mPaused(false), mVsync(true) {
	if (mActorMutex == nullptr) {
		SDL_Log("Failed to create actor mutex: %s", SDL_GetError());

		ERROR_BOX("Failed to create mutex");

		throw std::runtime_error("Failed to create mutex");
	}

	mUIScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	if (mUIScale == 0.0f) {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get display context scale: %s\033[0m",
			     SDL_GetError());

		mUIScale = 1.0f;
	}

	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
	} else {
		mBasePath = "";

		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mFailed to get base path: %s\033[0m", SDL_GetError());
	}

	mTextures = std::make_unique<TextureManager>(mBasePath);
	mShaders = std::make_unique<ShaderManager>(mBasePath);
	mEventManager = std::make_unique<EventManager>(this);

	mRenderer = new Renderer(this);
	mRenderer->swapWindow();

	mFontManager = new FontManager(mBasePath, this);
	mFontManager->loadFont("NotoSans.ttf");
	// mFontManager->loadFont("NotoSansSC-VF.ttf");

	// TODO: Icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_backpackShaderroySurface(icon);
	*/

	SDL_GL_SetSwapInterval(static_cast<int>(mVsync));

	setup();
}

void Game::setup() {
	SDL_Log("Setting up UIs");

#ifdef ANDROID
	new ControlUI(this);
#endif
	new MainMenu(this);

	SDL_Log("Successfully initialized OpenGL and UI\n");

	SDL_Log("Setting up game");

	new World(this);
	new Cube(this);
	new Sun(this);
	new Player(this);

	SDL_Log("Successfully initialized Game World");

	// SDL_CreateThread(initWorld, "Game setup", this);

	mTicks = SDL_GetTicks();

	mRenderer->setDemensions(getWidth(), getHeight());
}

/*
int Game::initWorld(void* gameptr) {
	SDL_Log("Setting up game");

	Game* game = static_cast<Game*>(gameptr);

	new World(game);
	new Cube(game);
	new Sun(game);
	new Player(game);

	SDL_Log("Successfully initialized Game World");

	return 0;
}
*/

int Game::iterate() {
	gui();
	input();
	update();
	draw();

#ifdef DEBUG
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid enum\033[0m");
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid value\033[0m");
				break;
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid operation\033[0m");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Invalid framebuffer op\033[0m");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\x1B[31mGLError: Out of memory\033[0m");
				break;
		}
	}
#endif

	return 0;
}

void Game::input() {
	for (const auto& ui : mUI) {
		ui->processInput(mEventManager->getKeystate());
	}

	if (!mPaused) {
		for (const auto& actor : mActors) {
			actor->input(mEventManager->getKeystate());
		}
	}
}

void Game::update() {
	// Update the game
	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05f) {
		delta = 0.05f;

		SDL_Log("Delta > 0.5f, cutting frame short");
	}
	mTicks = SDL_GetTicks();

	if (!mPaused) {
		// Update the Actors if not paused
		for (const auto& actor : mActors) {
			actor->update(delta);
		}
	}

	// Append the pending actors
	SDL_LockMutex(mActorMutex);
	std::copy(mPendingActors.begin(), mPendingActors.end(), std::back_inserter(mActors));
	mPendingActors.clear();
	SDL_UnlockMutex(mActorMutex);

	// Remove the dead Actors
	std::vector<const Actor*> deadActors;
	std::copy_if(mActors.begin(), mActors.end(), std::back_inserter(deadActors),
		     [](const Actor* actor) { return (actor->getState() == Actor::DEAD); });

	// Delete all the dead actors
	for (const auto& actor : deadActors) {
		delete actor;
	}

	for (const auto& ui : mUI) {
		if (ui->getState() == UIScreen::Active) {
			ui->update(delta);
		}
	}
}

void Game::gui() {
#ifdef IMGUI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	static bool wireframe = false;

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("%.3f ms %.1f FPS", (1000.f / io.Framerate), io.Framerate);
		ImGui::Text("%d Vsync", mVsync);

		/*
		Player* p = nullptr;
		for (const auto& actor : mActors) {
			if (dynamic_cast<Player*>(actor) != nullptr) {
				p = dynamic_cast<Player*>(actor);
				break;
			}
		}
		if (p != nullptr) {
			auto pos = p->getPosition();
			ImGui::Text("Player position: %dx%dx%d", static_cast<int>(pos.x()), static_cast<int>(pos.y()),
				    static_cast<int>(pos.z()));
		}
		*/

		if (ImGui::Button("VSync")) {
			mVsync = !mVsync;
			SDL_GL_SetSwapInterval(static_cast<int>(mVsync));
		}
		ImGui::Checkbox("Wireframe", &wireframe);

		ImGui::End();
	}

#ifndef GLES
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
#endif
}

void Game::draw() { mRenderer->draw(); }

int Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);

	ImGuiIO& io = ImGui::GetIO();

	switch (event.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_MOTION:
		case SDL_EVENT_MOUSE_WHEEL:
			if (io.WantCaptureMouse) {
				return 0;
			}

			break;

		case SDL_EVENT_TEXT_INPUT:
			if (io.WantTextInput) {
				return 0;
			}

			break;

		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
		case SDL_EVENT_GAMEPAD_ADDED:
		case SDL_EVENT_GAMEPAD_REMOVED:
			if (io.WantCaptureKeyboard) {
				return 0;
			}

			break;

		default:
			break;
	}
#endif

	return mEventManager->manageEvent(event);
}

void Game::addActor(Actor* actor) {
	// Everything goes to pending
	mPendingActors.emplace_back(actor);
}

void Game::removeActor(Actor* actor) {
	// NOTE: the pending actors was first
	auto iter = std::find(mActors.begin(), mActors.end(), actor);
	[[likely]] if (iter != mActors.end()) {
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}

	iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	[[likely]] if (iter != mPendingActors.end()) {
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}
}

void Game::removeUI(UIScreen* ui) {
	const auto iter = std::find(mUI.begin(), mUI.end(), ui);
	[[likely]] if (iter != mUI.end()) {
		std::iter_swap(iter, mUI.end() - 1);
		mUI.pop_back();
	}
}

Texture* Game::getTexture(const std::string& name) { return mTextures->get(name); }

Shader* Game::getShader(const std::string& vert, const std::string& frag, const std::string& geom) {
	return mShaders->get(vert, frag, geom);
}

Game::~Game() {
	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	while (!mActors.empty()) {
		delete mActors.back();
	}
	while (!mPendingActors.empty()) {
		delete mPendingActors.back();
	}

	delete mRenderer;

	SDL_DestroyMutex(mActorMutex);
}

[[nodiscard]] int Game::getWidth() const { return mRenderer->getWidth(); }
[[nodiscard]] int Game::getHeight() const { return mRenderer->getHeight(); }

void Game::setKey(const size_t key, const bool val) { mEventManager->setKey(key, val); }

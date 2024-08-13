#include "game.hpp"

#include "actors/actor.hpp"
#include "actors/cube.hpp"
#include "actors/player.hpp"
#include "actors/sun.hpp"
#include "actors/world.hpp"
#include "managers/shaderManager.hpp"
#include "managers/textureManager.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
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
	: mTextures(nullptr), mShaders(nullptr), mRenderer(nullptr), mActorMutex(SDL_CreateMutex()), mTicks(0),
	  mBasePath(""), mPaused(false), mVsync(true) {
	if (mActorMutex == nullptr) {
		SDL_Log("Failed to create actor mutex: %s", SDL_GetError());

		ERROR_BOX("Failed to create mutex");

		throw std::runtime_error("Failed to create mutex");
	}

	const char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
	} else {
		mBasePath = ""; // std::string(".") + SEPARATOR;
		SDL_Log("Unable to get base path: %s", SDL_GetError());
	}

#ifdef DEBUG
	SDL_Log(R"(
============================================================================================================================================================
 $$$$$$\                                       $$$$$$\                                          $$$$$$$$\                     $$\                     
$$  __$$\                                     $$  __$$\                                         $$  _____|                    \__|                    
$$ /  \__|$$\   $$\  $$$$$$\   $$$$$$\        $$ /  \__| $$$$$$\  $$$$$$\$$$$\   $$$$$$\        $$ |      $$$$$$$\   $$$$$$\  $$\ $$$$$$$\   $$$$$$\  
$$ |      $$ |  $$ | \____$$\ $$  __$$\       $$ |$$$$\  \____$$\ $$  _$$  _$$\ $$  __$$\       $$$$$\    $$  __$$\ $$  __$$\ $$ |$$  __$$\ $$  __$$\ 
$$ |      $$ |  $$ | $$$$$$$ |$$ /  $$ |      $$ |\_$$ | $$$$$$$ |$$ / $$ / $$ |$$$$$$$$ |      $$  __|   $$ |  $$ |$$ /  $$ |$$ |$$ |  $$ |$$$$$$$$ |
$$ |  $$\ $$ |  $$ |$$  __$$ |$$ |  $$ |      $$ |  $$ |$$  __$$ |$$ | $$ | $$ |$$   ____|      $$ |      $$ |  $$ |$$ |  $$ |$$ |$$ |  $$ |$$   ____|
\$$$$$$  |\$$$$$$$ |\$$$$$$$ |\$$$$$$  |      \$$$$$$  |\$$$$$$$ |$$ | $$ | $$ |\$$$$$$$\       $$$$$$$$\ $$ |  $$ |\$$$$$$$ |$$ |$$ |  $$ |\$$$$$$$\ 
 \______/  \____$$ | \_______| \______/        \______/  \_______|\__| \__| \__| \_______|      \________|\__|  \__| \____$$ |\__|\__|  \__| \_______|
          $$\   $$ |                                                                                                $$\   $$ |                        
          \$$$$$$  |                                                                                                \$$$$$$  |                        
           \______/                                                                                                  \______/                         
===========================================================================================================================================================
)");
#endif

	mTextures = std::make_unique<TextureManager>(mBasePath);
	mShaders = std::make_unique<ShaderManager>(mBasePath);

	mRenderer = new Renderer(this);

	// TODO: Icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_backpackShaderroySurface(icon);
	*/

	mRenderer->setWindowRelativeMouseMode(0);

	SDL_GL_SetSwapInterval(static_cast<int>(mVsync));

	mKeys = new bool[SDL_NUM_SCANCODES];

	for (size_t i = 0; i < SDL_NUM_SCANCODES; ++i) {
		mKeys[i] = false;
	}

	setup();
}

void Game::setup() {
	SDL_Log("Setting up UIs");

#ifdef ANDROID
	new ControlUI(this);
#endif
	// new MainMenu(this);

	SDL_Log("Successfully initialized OpenGL and UI\n");

	// NOTE: Camera must be initialized
	new Player(this);

	initWorld();

	// SDL_CreateThread(std::bind(&Game::initWorld, this), "", (void*)nullptr);
	// auto fn = std::function<class Fp>(std::bind(&Game::initWorld, this));
	// SDL_CreateThread(launchThreadHelper, "", static_cast<void*>(&fn));

	mTicks = SDL_GetTicks();
}

void Game::initWorld() {
	SDL_Log("Setting up game");

	new World(this);
	new Cube(this);
	new Sun(this);

	SDL_Log("Successfully initialized Game World");
}

int Game::iterate() {
#ifdef HOT
	if (std::filesystem::last_write_time(fullPath("shaders")) != last_time) {
		last_time = std::filesystem::last_write_time(fullPath("shaders"));

		mShaders->reload();
	}
#endif

	gui();
	input();
	update();
	draw();

#ifdef DEBUG
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				SDL_Log("GLError: Invalid enum");
				break;
			case GL_INVALID_VALUE:
				SDL_Log("GLError: Invalid value");
				break;
			case GL_INVALID_OPERATION:
				SDL_Log("GLError: Invalid operation");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SDL_Log("GLError: Invalid framebuffer op");
				break;
			case GL_OUT_OF_MEMORY:
				SDL_Log("GLError: Out of memory");
				break;
		}
	}
#endif

	return 0;
}

void Game::input() {
	for (const auto& ui : mUI) {
		ui->processInput(mKeys);
	}

	if (!mPaused) {
		for (const auto& actor : mActors) {
			actor->input(mKeys);
		}
	}
}

void Game::update() {
	// Update the game
	float delta = static_cast<float>(SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05f) {
		delta = 0.05f;

		SDL_Log("Delta > 0.5f, skipping");
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
		[[likely]] if (ui->getState() == UIScreen::Active) { ui->update(delta); }
	}
}

void Game::gui() {
#ifdef IMGUI
	static bool wireframe = false;

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("%.3f ms %.1f FPS", (1000.f / io.Framerate), io.Framerate);

		/*
		Player* p = nullptr;
		for (const auto& actor : mActors) {
			if (dynamic_cast<Player*>(actor) != nullptr) {
				p = dynamic_cast<Player*>(actor);
				break;
			}
		}
		if (p == nullptr) {
			auto pos = p->getPosition();
			ImGui::Text("Player position: %dx%dx%d", static_cast<int>(pos.x()), static_cast<int>(pos.y()),
				    static_cast<int>(pos.z()));
		}
		*/

		ImGui::Checkbox("VSync", &mVsync);
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

	SDL_GL_SetSwapInterval(static_cast<int>(mVsync));
#endif
}

void Game::draw() { mRenderer->draw(); }

int Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);
#endif

	static bool rel = true;

	switch (event.type) {
		case SDL_EVENT_QUIT: {
			return 1;
		}

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
			return 1;

			break;
		}

		case SDL_EVENT_KEY_DOWN: {
			switch (event.key.key) {
				case SDLK_ESCAPE: {
					MainMenu* menu = nullptr;
					for (const auto& element : mUI) {
						menu = dynamic_cast<MainMenu*>(element);

						if (menu != nullptr) {
							break;
						}
					}

					if (menu == nullptr) {
						// No main menu present
						new MainMenu(this);
					}
				}
				case SDLK_F1: {
					rel = !rel;
					mRenderer->setWindowRelativeMouseMode(static_cast<int>(rel));

					break;
				}
				case SDLK_F2: {
					if (mPaused) {
						mPaused = false;
					}

					// TODO: Maybe textures
					// mTextures->reload(true);
					mShaders->reload(true);
					mRenderer->reload();

					break;
				}
				case SDLK_F3: {
					mPaused = !mPaused;

					mTicks = SDL_GetTicks();

					break;
				}
				case SDLK_F5: {
					mVsync = !mVsync;

					SDL_GL_SetSwapInterval(static_cast<int>(mVsync));

					break;
				}
			}

			mKeys[event.key.scancode] = true;

			break;
		}
		case SDL_EVENT_KEY_UP: {
			mKeys[event.key.scancode] = false;

			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			mRenderer->setDemensions(event.window.data1, event.window.data2);

			break;
		}

		case SDL_EVENT_FINGER_DOWN: {
			for (const auto& ui : mUI) {
				ui->touch(event.tfinger.fingerID, event.tfinger.x * getWidth(),
					  getHeight() - event.tfinger.y * getHeight(), false);
			}

			break;
		}

		case SDL_EVENT_FINGER_UP: {
			for (const auto& ui : mUI) {
				ui->touch(event.tfinger.fingerID, event.tfinger.x * getWidth(),
					  getHeight() - event.tfinger.y * getHeight(), true);
			}

			break;
		}

		[[likely]] default:
			break;
	}

	return 0;
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

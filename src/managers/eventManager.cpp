#include "managers/eventManager.hpp"

#include "game.hpp"
#include "opengl/renderer.hpp"
#include "ui/mainMenu.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

EventManager::EventManager(class Game* game) : mGame(game) {
	mKeys = new bool[SDL_NUM_SCANCODES];

	for (size_t i = 0; i < SDL_NUM_SCANCODES; ++i) {
		mKeys[i] = false;
	}
}

int EventManager::manageEvent(const SDL_Event& event) {
	// NOTE: Returns in switch!
	switch (event.type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_QUIT: {

			// TODO: SDL_EVENT_LOW_MEMORY
			return 1;

			break;
		}

			// TODO: SDL_EVENT_LOW_MEMORY

		case SDL_EVENT_KEY_DOWN: {
			if (manageKeyboardEvent(event) != 0) {
				return 1;
			}

			mKeys[event.key.scancode] = true;

			break;
		}

		case SDL_EVENT_KEY_UP: {
			mKeys[event.key.scancode] = false;

			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			mGame->getRenderer()->setDemensions(event.window.data1, event.window.data2);

			break;
		}

		case SDL_EVENT_FINGER_DOWN: {
			for (const auto& ui : mGame->getUIs()) {
				ui->touch(event.tfinger.fingerID, event.tfinger.x * mGame->getWidth(),
					  mGame->getHeight() - event.tfinger.y * mGame->getHeight(), false);
			}

			break;
		}

		case SDL_EVENT_FINGER_UP: {
			for (const auto& ui : mGame->getUIs()) {
				ui->touch(event.tfinger.fingerID, event.tfinger.x * mGame->getWidth(),
					  mGame->getHeight() - event.tfinger.y * mGame->getHeight(), true);
			}

			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			for (const auto& ui : mGame->getUIs()) {
				ui->touch(event.button.which, event.button.x, mGame->getHeight() - event.button.y,
					  false);
			}

			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_UP: {
			for (const auto& ui : mGame->getUIs()) {
				ui->touch(event.button.which, event.button.x, mGame->getHeight() - event.button.y,
					  true);
			}

			break;
		}

		[[likely]] default:
			break;
	}

	return 0;
}

int EventManager::manageKeyboardEvent(const SDL_Event& event) {
	static bool rel = true;

	switch (event.key.key) {
		case SDLK_ESCAPE: {
			MainMenu* menu = nullptr;
			for (const auto& element : mGame->getUIs()) {
				menu = dynamic_cast<MainMenu*>(element);

				if (menu != nullptr) {
					break;
				}
			}

			if (menu == nullptr) {
				// No main menu present
				new MainMenu(mGame);
			}
			break;
		}

		case SDLK_F1: {
			rel = !rel;
			mGame->getRenderer()->setWindowRelativeMouseMode(static_cast<int>(rel));

			break;
		}
		/*
		 * FIXME:
		 *
		case SDLK_F2: {
			// TODO: Maybe reload textures

			// mTextures->reload();
			mShaders->reload();
			mRenderer->reload();

			break;
		}
		*/
		case SDLK_F3: {
			mGame->setPause(!mGame->isPaused());

			break;
		}

		default:
			break;
	}

	return 0;
}

#include "managers/eventManager.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"

#include <SDL3/SDL.h>

EventManager::EventManager(class Game* game) : mGame(game) {
	// TODO: Use std::array and std::span
	mKeys = new bool[SDL_SCANCODE_COUNT];

	for (size_t i = 0; i < SDL_SCANCODE_COUNT; ++i) {
		mKeys[i] = false;
	}
}

EventManager::~EventManager() { delete[] mKeys; }

SDL_AppResult EventManager::manageEvent(const SDL_Event& event) {
	// NOTE: Returns in switch!
	switch (event.type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_QUIT: {
			SDL_Log("Recieved quit event.");

			return SDL_APP_SUCCESS;
		}

			// TODO: SDL_EVENT_LOW_MEMORY

		case SDL_EVENT_KEY_DOWN: {
			if (manageKeyboardEvent(event) != SDL_APP_CONTINUE) {
				return SDL_APP_FAILURE;
			}

			mKeys[event.key.scancode] = true;

			// Debugging logs for bluetooth keyboard
			// Left: 80
			// Right: 79
			// Up: 82
			// Up, right, left, no right: 74!? ()
			// Up, left, right, no lect: 77??? (End)
			// Macos: Normal....
			// Bruh my bluetooth keyboard is bad :(
			SDL_Log("Key %d pressed", event.key.scancode);

			break;
		}

		case SDL_EVENT_KEY_UP: {
			mKeys[event.key.scancode] = false;

			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			//	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
			mGame->setDemensions(event.window.data1, event.window.data2);

			break;
		}

			// TODO: Fingers

			/*
			case SDL_EVENT_FINGER_DOWN: {
				const Eigen::Vector2f dimensions = mGame->getDemensions();
				for (const auto& ui : mGame->getUIs()) {
					ui->touch(event.tfinger.fingerID, event.tfinger.x * dimensions.x(),
						  dimensions.y() - event.tfinger.y * dimensions.y(), false);
				}

				break;
			}

			case SDL_EVENT_FINGER_UP: {
				const Eigen::Vector2f dimensions = mGame->getDemensions();
				for (const auto& ui : mGame->getUIs()) {
					ui->touch(event.tfinger.fingerID, event.tfinger.x * dimensions.x(),
						  dimensions.y() - event.tfinger.y * dimensions.y(), true);
				}

				break;
			}
			*/

		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			if (event.button.button == SDL_BUTTON_LEFT) {
				mGame->getSystemManager()->registerClick(event.button.x, event.button.y);
			}

			break;
		}

		[[likely]] default:
			break;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult EventManager::manageKeyboardEvent(const SDL_Event& event) {
	switch (event.key.key) {
		case SDLK_F3: {
			mGame->setPause(!mGame->isPaused());

			break;
		}

		default:
			break;
	}

	return SDL_APP_CONTINUE;
}

#include "managers/eventManager.hpp"

#include "game.hpp"
#include "third_party/Eigen/Core"
#include "ui/screens/mainUI.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

EventManager::EventManager(class Game* game) : mGame(game) {
	mKeys = new bool[SDL_NUM_SCANCODES];

	for (size_t i = 0; i < SDL_NUM_SCANCODES; ++i) {
		mKeys[i] = false;
	}
}

EventManager::~EventManager() { delete[] mKeys; }

SDL_AppResult EventManager::manageEvent(const SDL_Event& event) {
	// NOTE: Returns in switch!
	switch (event.type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_QUIT: {
			return SDL_APP_SUCCESS;
		}

			// TODO: SDL_EVENT_LOW_MEMORY

		case SDL_EVENT_KEY_DOWN: {
			if (manageKeyboardEvent(event) != SDL_APP_CONTINUE) {
				// FIXME: SDL_APP_SUCCESS
				return SDL_APP_FAILURE;
			}

			mKeys[event.key.scancode] = true;

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

			case SDL_EVENT_MOUSE_BUTTON_DOWN: {
				const Eigen::Vector2f dimensions = mGame->getDemensions();
				for (const auto& ui : mGame->getUIs()) {
					ui->touch(static_cast<SDL_FingerID>(event.button.which), event.button.x,
						  static_cast<float>(dimensions.y()) - event.button.y, false);
				}

				break;
			}

			case SDL_EVENT_MOUSE_BUTTON_UP: {
				const Eigen::Vector2f dimensions = mGame->getDemensions();
				for (const auto& ui : mGame->getUIs()) {
					ui->touch(static_cast<SDL_FingerID>(event.button.which), event.button.x,
						  static_cast<float>(dimensions.y()) - event.button.y, true);
				}

				break;
			}
			*/

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

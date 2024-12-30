#include "managers/eventManager.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"
#include "scene.hpp"
#include "scenes/level.hpp"

#include <SDL3/SDL.h>

EventManager::EventManager(class Game* game) : mGame(game), mKeys({}) {}

EventManager::~EventManager() {}

SDL_AppResult EventManager::manageEvent(const SDL_Event& event) {
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

			break;
		}

		case SDL_EVENT_KEY_UP: {
			mKeys[event.key.scancode] = false;

			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			//	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
			mGame->getSystemManager()->setDemensions(event.window.data1, event.window.data2);

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
				mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_DOWN_SIGNAL) = true;
			}

			if (event.button.button == SDL_BUTTON_RIGHT) {
				mGame->getSystemManager()->registerClick(event.button.x, event.button.y);
				mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_DOWN_SIGNAL) = true;
			}

			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_UP: {
			if (event.button.button == SDL_BUTTON_LEFT) {
				mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_UP_SIGNAL) = true;
				mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_DOWN_SIGNAL) = false;
			}

			if (event.button.button == SDL_BUTTON_RIGHT) {
				mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_UP_SIGNAL) = true;
				mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_DOWN_SIGNAL) = false;
			}
			break;
		}

		default:
			break;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult EventManager::manageKeyboardEvent(const SDL_Event& event) {
	switch (event.key.key) {
		case SDLK_F3: {
			break;
		}

		default: {
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

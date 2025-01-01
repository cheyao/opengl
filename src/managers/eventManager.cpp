#include "managers/eventManager.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"
#include "scene.hpp"
#include "scenes/level.hpp"

#include <SDL3/SDL.h>
#include <cassert>
#include <iostream> // For optional logging

// Activation time in milliseconds to differentiate between "click" vs. "hold"
static constexpr Uint32 ACTIVATION_TIME = 200; 

// Constructor initializes member variables
EventManager::EventManager(Game* game)
    : mGame(game)
    , mKeys({})
    , mLeftClickDown(0)
    , mRightClickDown(0)
{
}

// Default destructor
EventManager::~EventManager()
{
}

////////////////////////////////////////////////////////////////////////////////
// manageEvent
//  - Handles a single SDL_Event and returns an SDL_AppResult signifying if we
//    should continue, exit, or handle failures gracefully.
////////////////////////////////////////////////////////////////////////////////
SDL_AppResult EventManager::manageEvent(const SDL_Event& event)
{
    switch (event.type)
    {
        // Quit-like events
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_QUIT:
        {
            SDL_Log("Received quit event.");
            // Potentially set some global game->SetRunning(false) or similar
            return SDL_APP_SUCCESS; // or SDL_APP_FAILURE if you want to shut down
        }

        // Key down
        case SDL_EVENT_KEY_DOWN:
        {
            if (manageKeyboardEvent(event) != SDL_APP_CONTINUE)
            {
                // Something handled it in a way that indicates we should fail or exit
                return SDL_APP_FAILURE;
            }
            // Mark the scancode as pressed
            mKeys[event.key.scancode] = true;
            break;
        }

        // Key up
        case SDL_EVENT_KEY_UP:
        {
            mKeys[event.key.scancode] = false;
            break;
        }

        // Window resize event
        case SDL_EVENT_WINDOW_RESIZED:
        {
            // event.window.data1, event.window.data2 store the new width/height
            if (mGame && mGame->getSystemManager())
            {
                mGame->getSystemManager()->setDemensions(
                    event.window.data1, 
                    event.window.data2
                );
            }
            break;
        }

        // Mouse button down
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mLeftClickDown = SDL_GetTicks();
            }
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                mRightClickDown = SDL_GetTicks();
            }
            break;
        }

        // Mouse button up
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            Uint32 now = SDL_GetTicks();

            if (event.button.button == SDL_BUTTON_LEFT)
            {
                // If the time since press is less than threshold, treat as a click
                if (now - mLeftClickDown < ACTIVATION_TIME)
                {
                    // Mark the left-click-down signal
                    if (mGame && mGame->getLevel() && mGame->getLevel()->getScene())
                    {
                        mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_DOWN_SIGNAL) = true;
                    }
                }
                // Reset the timestamp
                mLeftClickDown = 0;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                if (now - mRightClickDown < ACTIVATION_TIME)
                {
                    if (mGame && mGame->getLevel() && mGame->getLevel()->getScene())
                    {
                        mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_DOWN_SIGNAL) = true;
                    }
                }
                mRightClickDown = 0;
            }
            break;
        }

        default:
            // Possibly handle other events or simply ignore them
            break;
    }

    return SDL_APP_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// update
//  - Called each frame to handle "hold" logic for mouse buttons, i.e., whether
//    the user is continuing to press a mouse button beyond the short-press time.
////////////////////////////////////////////////////////////////////////////////
void EventManager::update()
{
    int x = 0, y = 0;
    const Uint32 buttons = SDL_GetMouseState(&x, &y);

    // Check left mouse hold
    {
        Uint32 now = SDL_GetTicks();
        bool isLeftDown = (buttons & SDL_BUTTON_LMASK) != 0;
        bool isShortPressTime = (now - mLeftClickDown <= ACTIVATION_TIME);

        // If the button is held beyond the threshold, mark as hold
        if (isLeftDown && !isShortPressTime)
        {
            if (mGame && mGame->getLevel() && mGame->getLevel()->getScene())
            {
                mGame->getLevel()->getScene()->getSignal(LEFT_HOLD_SIGNAL) = mLeftClickDown;
                // We ensure the short-click signal is not triggered anymore
                mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_DOWN_SIGNAL) = false;
            }
        }
        else
        {
            // Not holding
            if (mGame && mGame->getLevel() && mGame->getLevel()->getScene())
            {
                mGame->getLevel()->getScene()->getSignal(LEFT_HOLD_SIGNAL) = false;
            }
        }
    }

    // Check right mouse hold
    {
        Uint32 now = SDL_GetTicks();
        bool isRightDown = (buttons & SDL_BUTTON_RMASK) != 0;
        bool isShortPressTime = (now - mRightClickDown <= ACTIVATION_TIME);

        if (isRightDown && !isShortPressTime)
        {
            if (mGame && mGame->getLevel() && mGame->getLevel()->getScene())
            {
                mGame->getLevel()->getScene()->getSignal(RIGHT_HOLD_SIGNAL) = mRightClickDown;
                mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_DOWN_SIGNAL) = false;
            }
        }
        else
        {
            if (mGame && mGame->getLevel() && mGame->getLevel()->getScene())
            {
                mGame->getLevel()->getScene()->getSignal(RIGHT_HOLD_SIGNAL) = false;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// manageKeyboardEvent
//  - Handles additional logic for particular key events. Return SDL_APP_CONTINUE
//    by default, or SDL_APP_FAILURE/SDL_APP_SUCCESS if you want special handling.
////////////////////////////////////////////////////////////////////////////////
SDL_AppResult EventManager::manageKeyboardEvent(const SDL_Event& event)
{
    switch (event.key.key)
    {
        case SDLK_F3:
        {
            // Example debug toggle
            // e.g. toggle debug overlay or something
            // ...
            break;
        }

        default:
        {
            // For unhandled keys
            break;
        }
    }

    return SDL_APP_CONTINUE;
}

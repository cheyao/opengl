#include "managers/eventManager.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"
#include "scene.hpp"
#include "scenes/level.hpp"

#include <SDL3/SDL.h>

// TODO: Run clang tidy here
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
            
            return SDL_APP_SUCCESS; // SDL_APP_SUCCESS makes the app shutdown with exit code 0
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
            mGame->getSystemManager()->setDemensions(
                event.window.data1, 
                event.window.data2
            );
            break;
        }

        // TODO: Handle fingers
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
            std::uint64_t now = SDL_GetTicks();

            if (event.button.button == SDL_BUTTON_LEFT)
            {
                // If the time since press is less than threshold, treat as a click
                if (now - mLeftClickDown < ACTIVATION_TIME)
                {
                    // Mark the left-click-down signal
                    
                    mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_DOWN_SIGNAL) = true;
                }
                // Reset the timestamp
                mLeftClickDown = 0;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                if (now - mRightClickDown < ACTIVATION_TIME)
                {
                    mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_DOWN_SIGNAL) = true;
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
    const auto buttons = SDL_GetMouseState(nullptr, nullptr);
    const std::uint64_t now = SDL_GetTicks();
    const bool isShortPressTime = (now - mLeftClickDown <= ACTIVATION_TIME);

    // Check left mouse hold
    {
        const bool isLeftDown = buttons & SDL_BUTTON_LMASK;

        // If the button is held beyond the threshold, mark as hold
        if (isLeftDown && !isShortPressTime)
        {
                mGame->getLevel()->getScene()->getSignal(LEFT_HOLD_SIGNAL) = mLeftClickDown;
                // We ensure the short-click signal is not triggered anymore
                mGame->getLevel()->getScene()->getSignal(LEFT_CLICK_DOWN_SIGNAL) = false;
        } else {
            // Not holding
            mGame->getLevel()->getScene()->getSignal(LEFT_HOLD_SIGNAL) = false;
        }
    }

    // Check right mouse hold
    {
        const bool isRightDown = buttons & SDL_BUTTON_RMASK
        const bool isShortPressTime = (now - mRightClickDown <= ACTIVATION_TIME);

        if (isRightDown && !isShortPressTime)
        {
            mGame->getLevel()->getScene()->getSignal(RIGHT_HOLD_SIGNAL) = mRightClickDown;
            mGame->getLevel()->getScene()->getSignal(RIGHT_CLICK_DOWN_SIGNAL) = false;
        }
        else
        {
            mGame->getLevel()->getScene()->getSignal(RIGHT_HOLD_SIGNAL) = false;
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
        default:
        {
            // For unhandled keys
            break;
        }
    }

    return SDL_APP_CONTINUE;
}

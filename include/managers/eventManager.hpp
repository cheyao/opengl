#pragma once

#include <SDL3/SDL.h>
#include <cstddef>
#include <array>
#include <span>

class EventManager {
      public:
	explicit EventManager(class Game* game);
	EventManager(EventManager&&) = delete;
	EventManager(const EventManager&) = delete;
	EventManager& operator=(EventManager&&) = delete;
	EventManager& operator=(const EventManager&) = delete;
	~EventManager();

	[[nodiscard]] SDL_AppResult manageEvent(const union SDL_Event& event);

	void setKey(const std::size_t key, const bool val) { mKeys[key] = val; }
	[[nodiscard]] std::span<bool> getKeystate() { return mKeys; };

      private:
	[[nodiscard]] SDL_AppResult manageKeyboardEvent(const union SDL_Event& event);

	class Game* mGame;

	std::array<bool, SDL_SCANCODE_COUNT> mKeys;
};

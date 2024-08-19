#pragma once

#include <SDL3/SDL.h>
#include <cstddef>

class EventManager {
      public:
	explicit EventManager(class Game* game);
	EventManager(EventManager&&) = delete;
	EventManager(const EventManager&) = delete;
	EventManager& operator=(EventManager&&) = delete;
	EventManager& operator=(const EventManager&) = delete;
	~EventManager();

	[[nodiscard]] SDL_AppResult manageEvent(const union SDL_Event& event);

	[[nodiscard]] bool* getKeystate() { return mKeys; };

	void setKey(const size_t key, const bool val) { mKeys[key] = val; }

      private:
	[[nodiscard]] SDL_AppResult manageKeyboardEvent(const union SDL_Event& event);

	class Game* mGame;

	bool* mKeys;
};

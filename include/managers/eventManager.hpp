#pragma once

#include <cstddef>

class EventManager {
      public:
	explicit EventManager(class Game* game);
	EventManager(EventManager&&) = delete;
	EventManager(const EventManager&) = delete;
	EventManager& operator=(EventManager&&) = delete;
	EventManager& operator=(const EventManager&) = delete;
	~EventManager() = default;

	[[nodiscard]] int manageEvent(const union SDL_Event& event);

	[[nodiscard]] bool* getKeystate() { return mKeys; };

	void setKey(const size_t key, const bool val) { mKeys[key] = val; }

      private:
	[[nodiscard]] int manageKeyboardEvent(const union SDL_Event& event);

	class Game* mGame;

	bool* mKeys;
};

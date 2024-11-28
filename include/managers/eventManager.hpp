#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <cstddef>
#include <span>

#include "opengl/shader.hpp"

class EventManager {
      public:
	constexpr const static inline auto LEFT_CLICK_DOWN_SIGNAL = "left_down_signal"_u;
	constexpr const static inline auto LEFT_CLICK_UP_SIGNAL = "left_up_signal"_u;

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

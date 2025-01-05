#pragma once

#include <SDL3/SDL.h>
#include <cstddef>

#include "opengl/shader.hpp"

class EventManager {
      public:
	constexpr const static inline auto LEFT_CLICK_DOWN_SIGNAL = "left_click"_u;
	constexpr const static inline auto LEFT_HOLD_SIGNAL = "left_hold"_u;
	constexpr const static inline auto RIGHT_CLICK_DOWN_SIGNAL = "right_click"_u;
	constexpr const static inline auto RIGHT_HOLD_SIGNAL = "right_hold"_u;

	explicit EventManager() noexcept;
	EventManager(EventManager&&) = delete;
	EventManager(const EventManager&) = delete;
	EventManager& operator=(EventManager&&) = delete;
	EventManager& operator=(const EventManager&) = delete;
	~EventManager() = default;

	[[nodiscard]] SDL_AppResult manageEvent(const union SDL_Event& event);
	void update();

      private:
	// 0.2s
	constexpr const static inline std::uint64_t ACTIVATION_TIME = 220ull;
	[[nodiscard]] SDL_AppResult manageKeyboardEvent(const union SDL_Event& event);

	class Game* mGame;

	std::uint64_t mLeftClickDown;
	std::uint64_t mRightClickDown;
};

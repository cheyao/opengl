#pragma once

#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>

class UIComponent {
      public:
	explicit UIComponent(class UIScreen* owner, Eigen::Vector2f position);
	UIComponent(UIComponent&&) = delete;
	UIComponent(const UIComponent&) = delete;
	UIComponent& operator=(UIComponent&&) = delete;
	UIComponent& operator=(const UIComponent&) = delete;
	virtual ~UIComponent();

	virtual void update([[maybe_unused]] const float delta);
	virtual void input([[maybe_unused]] const bool* keystate);
	virtual void draw([[maybe_unused]] const class Shader* shader);
	virtual void drawText([[maybe_unused]] const class Shader* shader);
	virtual void touch(const SDL_FingerID& finger, const float x, const float y, const bool lift);

	Eigen::Vector2f getPosition() const { return mPosition; };
	void setPosition(Eigen::Vector2f pos) { mPosition = pos; }

      protected:
	class UIScreen* mOwner;

	Eigen::Vector2f mPosition;
};

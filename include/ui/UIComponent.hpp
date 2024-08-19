#pragma once

#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <string_view>
#include <string>

#define CENTER std::numeric_limits<float>::max()

class UIComponent {
      public:
	explicit UIComponent(class UIScreen* owner, const Eigen::Vector2f position, const std::string_view& name);
	UIComponent(UIComponent&&) = delete;
	UIComponent(const UIComponent&) = delete;
	UIComponent& operator=(UIComponent&&) = delete;
	UIComponent& operator=(const UIComponent&) = delete;
	virtual ~UIComponent();

	virtual void update([[maybe_unused]] const float delta);
	virtual void input([[maybe_unused]] const bool* keystate);
	virtual void draw([[maybe_unused]] class Shader* shader);
	virtual void drawText([[maybe_unused]] class Shader* shader);
	virtual void touch(const SDL_FingerID& finger, const float x, const float y, const bool lift);

	[[nodiscard]] Eigen::Vector2f getPosition() const { return mPosition; };
	void setPosition(const Eigen::Vector2f pos) { mPosition = pos; }

	[[nodiscard]] Eigen::Vector2f getSize() const { return mSize; }
	void setSize(const Eigen::Vector2f size) { mSize = size; }

	[[nodiscard]] const std::string& getName() const { return mName; }

      protected:
	class UIScreen* mOwner;
	const std::string mName;

	Eigen::Vector2f mPosition;
	Eigen::Vector2f mSize;
};

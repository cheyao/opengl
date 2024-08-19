#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"

#include <functional>
#include <SDL3/SDL.h>

class ButtonComponent : public UIComponent {
      public:
	// TODO: Resizable padding
	// TODO: Auto size when with text
	explicit ButtonComponent(class UIScreen* owner, class Texture* const texture, const Eigen::Vector2f position);
	ButtonComponent(ButtonComponent&&) = delete;
	ButtonComponent(const ButtonComponent&) = delete;
	ButtonComponent& operator=(ButtonComponent&&) = delete;
	ButtonComponent& operator=(const ButtonComponent&) = delete;
	~ButtonComponent() override;

	void draw(class Shader* shader) override;
	void touch(const SDL_FingerID& finger, const float x, const float y, const bool lift) override;

	void onClick(const std::function<void()>& func) { mOnClick = func; }
	void onRelease(const std::function<void()>& func) { mOnRelease = func; }

      private:
	class Mesh* mMesh;
	const int mWidth, mHeight;

	std::function<void()> mOnClick;
	std::function<void()> mOnRelease;

	SDL_FingerID mCapture;
};

#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"

#include <functional>

#define CENTER std::numeric_limits<float>::max()

class ButtonComponent : public UIComponent {
      public:
	// TODO: Resizable padding
	// TODO: Auto size when with text
	explicit ButtonComponent(class UIScreen* owner, class Texture* const texture, const Eigen::Vector2f padding);
	ButtonComponent(ButtonComponent&&) = delete;
	ButtonComponent(const ButtonComponent&) = delete;
	ButtonComponent& operator=(ButtonComponent&&) = delete;
	ButtonComponent& operator=(const ButtonComponent&) = delete;
	~ButtonComponent() override = default;

	void draw(const class Shader* shader) override;
	void touch(const SDL_FingerID& finger, const float x, const float y, const bool lift) override;

	void onClick(const std::function<void()> func) { mOnClick = func; }
	void onRelease(const std::function<void()> func) { mOnRelease = func; }

      private:
	class Mesh* mMesh;
	std::function<void()> mOnClick;
	std::function<void()> mOnRelease;
	const Eigen::Vector2f mPadding;

	SDL_FingerID mCapture;

	const int mWidth, mHeight;
};

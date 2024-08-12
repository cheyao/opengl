#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <functional>

class ButtonComponent : public UIComponent {
      public:
	// TODO: Resizable padding
	explicit ButtonComponent(class UIScreen* owner, class Texture* const texture, const Eigen::Vector2f padding);
	ButtonComponent(ButtonComponent&&) = delete;
	ButtonComponent(const ButtonComponent&) = delete;
	ButtonComponent& operator=(ButtonComponent&&) = delete;
	ButtonComponent& operator=(const ButtonComponent&) = delete;
	~ButtonComponent() override = default;

	void draw(const class Shader* shader) override;
	void touch(const SDL_FingerID& finger, const float x, const float y, const bool lift) override;

	void onClick(const std::function<void(class Game*)> func) { mOnClick = func; }
	void onRelease(const std::function<void(class Game*)> func) { mOnRelease = func; }

      private:
	class Mesh* mMesh;
	std::function<void(class Game*)> mOnClick;
	std::function<void(class Game*)> mOnRelease;
	const Eigen::Vector2f mPadding;

	SDL_FingerID mCapture;

	const int mWidth, mHeight;
};

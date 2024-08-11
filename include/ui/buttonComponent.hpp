#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <functional>

class ButtonComponent : public UIComponent {
      public:
	// TODO: Resizable padding
	explicit ButtonComponent(class UIScreen* owner, class Texture* const texture,
				 const std::function<void()>& onClick, const Eigen::Vector2f padding);
	ButtonComponent(ButtonComponent&&) = delete;
	ButtonComponent(const ButtonComponent&) = delete;
	ButtonComponent& operator=(ButtonComponent&&) = delete;
	ButtonComponent& operator=(const ButtonComponent&) = delete;
	~ButtonComponent() override = default;

	void draw(const class Shader* shader) override;

      private:
	class Mesh* mMesh;
	const std::function<void()> mOnClick;
	const Eigen::Vector2f mPadding;
};

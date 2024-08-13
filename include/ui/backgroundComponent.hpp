#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"

class BackgroundComponent : public UIComponent {
      public:
	explicit BackgroundComponent(class UIScreen* owner, const Eigen::Vector3f color);
	BackgroundComponent(BackgroundComponent&&) = delete;
	BackgroundComponent(const BackgroundComponent&) = delete;
	BackgroundComponent& operator=(BackgroundComponent&&) = delete;
	BackgroundComponent& operator=(const BackgroundComponent&) = delete;
	~BackgroundComponent() = default;

	void draw([[maybe_unused]] const class Shader* shader);

      private:
	Eigen::Vector3f mColor;
};

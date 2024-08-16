#pragma once

#include "ui/UIComponent.hpp"
#include "third_party/Eigen/Core"

#include <string>
#include <string_view>

#define CENTER std::numeric_limits<float>::max()

class TextComponent : public UIComponent {
      public:
	explicit TextComponent(class UIScreen* owner, std::string_view text, Eigen::Vector2f position);
	TextComponent(TextComponent&&) = delete;
	TextComponent(const TextComponent&) = delete;
	TextComponent& operator=(TextComponent&&) = delete;
	TextComponent& operator=(const TextComponent&) = delete;
	~TextComponent() = default;

	void drawText([[maybe_unused]] const class Shader* shader) override;

      private:
	std::string mText;
	Eigen::Vector2f mPosition;
};

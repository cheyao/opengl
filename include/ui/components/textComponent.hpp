#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"

#include <string>

#define CENTER std::numeric_limits<float>::max()

class TextComponent : public UIComponent {
      public:
	explicit TextComponent(class UIScreen* owner, const std::basic_string<char32_t>& text,
			       Eigen::Vector2f position);
	TextComponent(TextComponent&&) = delete;
	TextComponent(const TextComponent&) = delete;
	TextComponent& operator=(TextComponent&&) = delete;
	TextComponent& operator=(const TextComponent&) = delete;
	~TextComponent() = default;

	void drawText([[maybe_unused]] const class Shader* shader) override;

      private:
	const std::basic_string<char32_t> mText;
	Eigen::Vector2f mPosition;
};

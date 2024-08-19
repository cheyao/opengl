#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"

#include <string>

class TextComponent : public UIComponent {
      public:
	explicit TextComponent(class UIScreen* owner, const std::string& id,
			       Eigen::Vector2f position);
	TextComponent(TextComponent&&) = delete;
	TextComponent(const TextComponent&) = delete;
	TextComponent& operator=(TextComponent&&) = delete;
	TextComponent& operator=(const TextComponent&) = delete;
	~TextComponent() = default;

	void drawText([[maybe_unused]] const class Shader* shader) override;

      private:
	const std::string mTextID;
	Eigen::Vector2f mPosition;
};

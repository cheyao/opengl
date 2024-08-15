#pragma once

#include "ui/UIComponent.hpp"

#include <string>
#include <string_view>

class TextComponent : public UIComponent {
      public:
	explicit TextComponent(class UIScreen* owner, std::string_view text);
	TextComponent(TextComponent&&) = delete;
	TextComponent(const TextComponent&) = delete;
	TextComponent& operator=(TextComponent&&) = delete;
	TextComponent& operator=(const TextComponent&) = delete;
	~TextComponent() = default;

	void draw([[maybe_unused]] const class Shader* shader) override;

      private:
	std::string mText;
};

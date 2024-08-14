#pragma once

#include "ui/UIComponent.hpp"

class TextComponent : public UIComponent {
      public:
	explicit TextComponent(class UIScreen* owner);
	TextComponent(TextComponent&&) = delete;
	TextComponent(const TextComponent&) = delete;
	TextComponent& operator=(TextComponent&&) = delete;
	TextComponent& operator=(const TextComponent&) = delete;
	~TextComponent() = default;

      private:
};

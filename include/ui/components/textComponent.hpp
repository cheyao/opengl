#pragma once

#include "third_party/Eigen/Core"
#include "ui/UIComponent.hpp"

#include <string>

class TextComponent : public UIComponent {
      public:
	explicit TextComponent(class UIScreen* owner, class UIComponent* parent, const std::string& id,
			       const Eigen::Vector2f offset = Eigen::Vector2f::Zero());
	TextComponent(TextComponent&&) = delete;
	TextComponent(const TextComponent&) = delete;
	TextComponent& operator=(TextComponent&&) = delete;
	TextComponent& operator=(const TextComponent&) = delete;
	~TextComponent() = default;

	void drawText([[maybe_unused]] class Shader* shader) override;

      private:
	const std::string mTextID;
	const class UIComponent* mParent;
	const Eigen::Vector2f mOffset;
};

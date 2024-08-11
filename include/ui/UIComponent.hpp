#pragma once

class UIComponent {
      public:
	explicit UIComponent(class UIScreen* owner);
	UIComponent(UIComponent&&) = delete;
	UIComponent(const UIComponent&) = delete;
	UIComponent& operator=(UIComponent&&) = delete;
	UIComponent& operator=(const UIComponent&) = delete;
	virtual ~UIComponent();

	virtual void update([[maybe_unused]] const float delta);
	virtual void input([[maybe_unused]] const bool* keystate);
	// virtual void touch([[maybe_unused]] const bool* keystate);
	virtual void draw([[maybe_unused]] const class Shader* shader);

      private:
	class UIScreen* mOwner;
};

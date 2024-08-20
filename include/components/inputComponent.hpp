#pragma once

#include "components/component.hpp"

#include <functional>
#include <unordered_map>

class InputComponent : public Component {
      public:
	explicit InputComponent(class Actor* owner, int updateOrder = 10);
	InputComponent(InputComponent&&) = delete;
	InputComponent(const InputComponent&) = delete;
	InputComponent& operator=(InputComponent&&) = delete;
	InputComponent& operator=(const InputComponent&) = delete;
	~InputComponent() override = default;

	void addCalback(const size_t key, const std::function<void()>& func) { mCallbacks[key] = func; }
	void input(const bool* keystate) override;

      private:
	std::unordered_map<size_t, std::function<void()>> mCallbacks;
};

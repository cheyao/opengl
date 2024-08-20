#include "components/inputComponent.hpp"

#include "actors/actor.hpp"
#include "components/component.hpp"

InputComponent::InputComponent(class Actor* owner, int updateOrder) : Component(owner, updateOrder) {}

void InputComponent::input(const bool* keystate) {
	for (const auto& [key, func] : mCallbacks) {
		if (keystate[key]) {
			func();
		}
	}
}

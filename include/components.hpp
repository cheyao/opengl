#pragma once

#include "managers/entityManager.hpp"
#include "third_party/Eigen/Core"

#include <functional>

namespace Components {
struct position {
	Eigen::Vector2f pos;
};

struct texture {
	class Texture* texture;
	class Shader* shader = nullptr;
	float scale = 1.0f;
};

struct input {
	std::function<void(class Scene* scene, EntityID entity, const bool* scancodes, const float delta)> function;
};
} // namespace Components

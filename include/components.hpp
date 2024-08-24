#pragma once

#include "third_party/Eigen/Core"

namespace Components {
struct position {
	Eigen::Vector2f pos;
};

struct velocity {
	Eigen::Vector2f vel;
};

struct texture {
	class Texture* texture;
	class Shader* shader = nullptr;
	float scale = 1.0f;
};
} // namespace Components

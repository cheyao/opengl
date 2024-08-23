#pragma once

#include "third_party/Eigen/Core"

namespace Components {
struct position {
	Eigen::Vector2f pos;
};

struct velocity {
	Eigen::Vector2f vel;
};
} // namespace Components

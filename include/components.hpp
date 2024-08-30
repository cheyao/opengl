#pragma once

#include "managers/entityManager.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <functional>

namespace Components {
struct position {
	Eigen::Vector2f pos;
};

struct velocity {
	Eigen::Vector2f vel;
};

struct collision {
	Eigen::Vector2f offset;
	Eigen::Vector2f size;
	bool stationary = false;
};

struct texture {
	class Texture* texture;
	class Shader* shader = nullptr;
	float scale = 1.0f;
};

struct input {
	std::function<void(class Scene* scene, EntityID entity, const bool* scancodes, const float delta)> function;
};

struct text {
	std::string id;
	// Maybe offset
};

// Misc stuff like jumping
struct misc {
	enum {
		JUMP = 0b1,
		PLAYER = 0b10,
	};
	std::uint64_t what;
};

struct block {
	enum BlockType {
		AIR,
		GRASS,
		DIRT,
		STONE,
	} type;
};
} // namespace Components

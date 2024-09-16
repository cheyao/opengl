#pragma once

#include "managers/entityManager.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <functional>

// PERF: Grid blocks, convert to cords in shader

namespace Components {
struct position {
	position(const Eigen::Vector2f position) : mPosition(position) {}

	Eigen::Vector2f mPosition;
};

struct velocity {
	velocity(const Eigen::Vector2f velocity) : mVelocity(velocity) {}
	Eigen::Vector2f mVelocity;
};

struct collision {
	collision(const Eigen::Vector2f offset, const Eigen::Vector2f size, const bool stationary = false)
		: mOffset(offset), mSize(size), mStationary(stationary) {}

	Eigen::Vector2f mOffset;
	Eigen::Vector2f mSize;
	bool mStationary = false;
};

struct texture {
	texture(class Texture* t, const float s = 0.7f, class Shader* sh = nullptr)
		: mTexture(t), mScale(s), mShader(sh) {}

	class Texture* mTexture;
	float mScale = 0.7f;
	class Shader* mShader = nullptr;
};

struct input {
	std::function<void(class Scene* scene, EntityID entity, const bool* scancodes, const float delta)> mFunction;

	// I'm not writing it all again
	input(const decltype(mFunction) function) : mFunction(function) {}
};

struct text {
	text(const std::string id) : mID(id) {}

	std::string mID;
	// Maybe offset
};

// Misc stuff like jumping
struct misc {
	misc(const std::uint64_t what) : mWhat(what) {}

	enum {
		JUMP = 0b1,
		PLAYER = 0b10,
		CROSSHAIR = 0b100,
	};
	std::uint64_t mWhat;
};

struct block {
	enum BlockType {
		AIR,
		GRASS_BLOCK,
		DIRT,
		STONE,
	} mType;

	block(const decltype(mType) type) : mType(type) {}
};
} // namespace Components

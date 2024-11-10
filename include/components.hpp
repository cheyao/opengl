#pragma once

#include "managers/entityManager.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <functional>
#include <span>

// Android needs the constructers to be able to use emplace
class Texture;
class Shader;

namespace Components {
struct position {
	Eigen::Vector2f mPosition;

	position(const decltype(mPosition) pos) noexcept : mPosition(pos) {}
};

struct velocity {
	Eigen::Vector2f mVelocity;

	velocity(const decltype(mVelocity) vel) noexcept : mVelocity(vel) {}
};

struct collision {
	Eigen::Vector2f mOffset;
	Eigen::Vector2f mSize;
	bool mStationary = false;

	collision(const decltype(mOffset) offset, const decltype(mSize) size,
		  const decltype(mStationary) stationary = false) noexcept
		: mOffset(offset), mSize(size), mStationary(stationary) {}
};

struct texture {
	class Texture* mTexture;
	float mScale = 1.0f;
	class Shader* mShader = nullptr;

	texture(const decltype(mTexture) tex, const decltype(mScale) scale = 1.0f,
		const decltype(mShader) shader = nullptr) noexcept
		: mTexture(tex), mScale(scale), mShader(shader) {}
};

struct input {
	std::function<void(class Scene* scene, EntityID entity, const std::span<bool> scancodes, const float delta)>
		mFunction;

	input(const decltype(mFunction) function) noexcept : mFunction(function) {}
};

struct text {
	std::string mID;

	text(const decltype(mID) id) noexcept : mID(id) {}
};

// Misc stuff like jumping
struct misc {
	enum {
		JUMP = 0b1,
		PLAYER = 0b10,
		CROSSHAIR = 0b100,
	};
	std::uint64_t mWhat;
	void* container;

	constexpr misc(const decltype(mWhat) what) noexcept : mWhat(what) {}
};

struct block {
	enum BlockType : std::uint64_t {
		AIR = 0,
		GRASS_BLOCK,
		DIRT,
		STONE,
	} mType;
	Eigen::Vector2i mPosition;

	block(const decltype(mType) type, const decltype(mPosition) position) noexcept
		: mType(type), mPosition(position) {}

	constexpr const static inline auto BLOCK_SIZE = 112;
};
} // namespace Components

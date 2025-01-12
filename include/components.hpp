#pragma once

#include "managers/entityManager.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <functional>
#include <span>
#include <type_traits>
#include <vector>

// Android needs the constructers to be able to use emplace
class Texture;
class Shader;
class Inventory;

template <typename T>
concept isEnum = requires(T e) { std::is_enum_v<T>; };

template <typename T>
	requires isEnum<T>
constexpr auto etoi(T e) {
	return static_cast<std::underlying_type_t<T>>(e);
}

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
	float mScale;

	texture(const decltype(mTexture) tex, const float scale = 1.0f) noexcept : mTexture(tex), mScale(scale) {}
};

struct animated_texture {
	class Texture* mSpriteSheet;
	const Eigen::Vector2f mSize;
	unsigned int mSelect;

	animated_texture(const decltype(mSpriteSheet) spriteSheet, const decltype(mSize)& size, unsigned int select = 0)
		: mSpriteSheet(spriteSheet), mSize(size), mSelect(select) {}
};

struct input {
	void (*mFunction)(class Scene* scene, EntityID entity, const float delta);

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

	constexpr misc(const decltype(mWhat) what) noexcept : mWhat(what) {}
};

enum class Item : std::uint64_t;

struct block {
	Item mType;
	Eigen::Vector2i mPosition;
	bool mClose;

	block(const decltype(mType) type, const decltype(mPosition) position) noexcept
		: mType(type), mPosition(position) {}

	constexpr const static inline auto BLOCK_SIZE = 112;
};

struct inventory {
	class Inventory* mInventory;

	constexpr inventory(class Inventory* inv) noexcept : mInventory(inv) {}
};

struct item {
	Item mType;
};
} // namespace Components

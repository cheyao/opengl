#include "ui/components/buttonComponent.hpp"

#include "game.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "third_party/Eigen/Core"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <cmath>
#include <functional>
#include <vector>

ButtonComponent::ButtonComponent(UIScreen* owner, Texture* const texture, const Eigen::Vector2f padding)
	: UIComponent(owner, padding), mPadding(padding), mWidth(texture->getWidth()), mHeight(texture->getHeight()),
	  mOnClick(nullptr), mOnRelease(nullptr), mCapture(0) {
	const std::vector<float> vertices = {
		0.0f,
		0.0f,
		0.0f, // TL
		0.0f,
		static_cast<float>(mHeight),
		0.0f, // BL
		static_cast<float>(mWidth),
		0.0f,
		0.0f, // TR
		static_cast<float>(mWidth),
		static_cast<float>(mHeight),
		0.0f // BR
	};
	const std::vector<float> texturePos = {
		1.0f, 1.0f, // TR
		1.0f, 0.0f, // BR
		0.0f, 1.0f, // TL
		0.0f, 0.0f  // BL
	};
	const std::vector<GLuint> indices = {2, 1, 0,  // a
					     1, 2, 3}; // b
	const std::vector<std::pair<Texture* const, TextureType>> textures = {
		std::make_pair(texture, TextureType::DIFFUSE)};

	mMesh = new Mesh(vertices, {}, texturePos, indices, textures);
}

ButtonComponent::~ButtonComponent() { delete mMesh; }

// This is usually not a good practice, but trust me, I need it
// I defined a magic number for center, so I must use == to compare
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

void ButtonComponent::draw(const Shader* shader) {
	mSize = Eigen::Vector2f(mWidth * mOwner->getGame()->getScale(), mHeight * mOwner->getGame()->getScale());

	shader->activate();

	Eigen::Affine3f model = Eigen::Affine3f::Identity();
	float x = mPadding.x();
	float y = mPadding.y();

	assert(!std::isnan(x) && !std::isinf(x));
	assert(!std::isnan(y) && !std::isinf(y));

	if (x == CENTER) {
		x = static_cast<float>(mOwner->getGame()->getWidth()) / 2 -
		    static_cast<float>(mWidth) * mOwner->getGame()->getScale() / 2;
	} else if (x < 0) {
		x = mOwner->getGame()->getWidth() - static_cast<float>(mWidth) * mOwner->getGame()->getScale() + x;
	}

	if (y == CENTER) {
		y = static_cast<float>(mOwner->getGame()->getHeight()) / 2 -
		    (static_cast<float>(mHeight) * mOwner->getGame()->getScale()) / 2;
	} else if (y < 0) {
		y = mOwner->getGame()->getHeight() - static_cast<float>(mHeight) * mOwner->getGame()->getScale() + y;
	}

	model.translate(Eigen::Vector3f(x, y, 0.0f));
	model.scale(mOwner->getGame()->getScale());

	shader->set("model", model);

	mMesh->draw(shader);
}

void ButtonComponent::touch(const SDL_FingerID& finger, const float x, const float y, const bool lift) {
	// TODO: Cleanup code
	float buttonX = mPadding.x();
	float buttonY = mPadding.y();

	assert(!std::isnan(x) && !std::isinf(x));
	assert(!std::isnan(y) && !std::isinf(y));
	assert(!std::isnan(buttonX) && !std::isinf(buttonX));
	assert(!std::isnan(buttonY) && !std::isinf(buttonY));

	if (buttonX == CENTER) {
		buttonX = static_cast<float>(mOwner->getGame()->getWidth()) / 2 -
			  static_cast<float>(mWidth) * mOwner->getGame()->getScale() / 2;
	} else if (buttonX < 0) {
		buttonX = mOwner->getGame()->getWidth() - static_cast<float>(mWidth) * mOwner->getGame()->getScale() +
			  buttonX;
	}

	if (buttonY == CENTER) {
		buttonY = static_cast<float>(mOwner->getGame()->getHeight()) / 2 -
			  (static_cast<float>(mHeight) * mOwner->getGame()->getScale()) / 2;
	} else if (buttonY < 0) {
		buttonY = mOwner->getGame()->getHeight() - static_cast<float>(mHeight) * mOwner->getGame()->getScale() +
			  buttonY;
	}

	if ((buttonX <= x && x < (buttonX + mWidth * mOwner->getGame()->getScale())) &&
	    (buttonY <= y && y < (buttonY + mHeight * mOwner->getGame()->getScale()))) {
		if (lift == false) {
			if (mOnClick != nullptr) {
				mOnClick();
			}

			mCapture = finger;
		} else if (finger == mCapture) {
			if (mOnRelease != nullptr) {
				mOnRelease();
			}

			mCapture = 0;
		}
	}
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
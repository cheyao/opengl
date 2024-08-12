#include "ui/buttonComponent.hpp"

#include "game.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <cmath>
#include <functional>
#include <vector>

ButtonComponent::ButtonComponent(UIScreen* owner, Texture* const texture, const Eigen::Vector2f padding)
	: UIComponent(owner), mOnClick(nullptr), mOnRelease(nullptr), mPadding(padding), mCapture(0),
	  mWidth(texture->getWidth()), mHeight(texture->getHeight()) {

	const std::vector<float> vertices = {
		000.0f, 000.0f, -50.0f, // TL
		000.0f, 100.0f, -50.0f, // BL
		100.0f, 000.0f, -50.0f, // TR
		100.0f, 100.0f, -50.0f	// BR
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

void ButtonComponent::draw(const Shader* shader) {
	shader->activate();

	Eigen::Affine3f model = Eigen::Affine3f::Identity();
	float x = mPadding.x();
	float y = mPadding.y();

	if (std::isnan(x)) {
		x = static_cast<float>(mOwner->getGame()->getWidth()) / 2 - static_cast<float>(mWidth) / 2;
	}
	SDL_Log("%f", x);

	if (x < 0) {
		x += mOwner->getGame()->getWidth();
	}

	if (std::isnan(y)) {
		y = static_cast<float>(mOwner->getGame()->getHeight()) / 2 + static_cast<float>(mHeight) / 2;
	}

	y = -y;
	if (y < 0) {
		y += mOwner->getGame()->getHeight();
	}

	model.translate(Eigen::Vector3f(x, y, 0.0f));

	shader->set("model", model);

	mMesh->draw(shader);
}

void ButtonComponent::touch(const SDL_FingerID& finger, const float x, const float y, const bool lift) {
	float bx = mPadding.x();
	float by = mPadding.y();

	if (bx < 0) {
		bx += mOwner->getGame()->getWidth();
	}

	by = -by;
	if (by < 0) {
		by += mOwner->getGame()->getHeight();
	}

	if (!lift) {
		if ((bx <= x && x < (bx + mWidth)) && (by <= y && y < (by + mHeight))) {
			if (mOnClick != nullptr) {
				mOnClick();
			}

			mCapture = finger;
		}
	} else if (mCapture != 0 && finger == mCapture) {
		if ((bx <= x && x < (bx + mWidth)) && (by <= y && y < (by + mHeight))) {
			if (mOnRelease != nullptr) {
				mOnRelease();
			}

			mCapture = 0;
		}
	}
}

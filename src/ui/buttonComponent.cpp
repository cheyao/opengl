#include "ui/buttonComponent.hpp"

#include "game.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <functional>
#include <vector>

ButtonComponent::ButtonComponent(UIScreen* owner, Texture* const texture, const std::function<void()>& onClick,
				 const Eigen::Vector2f padding)
	: UIComponent(owner), mOnClick(onClick), mPadding(padding) {

	const std::vector<float> vertices = {
		+1.0f, +1.0f, +0.0f, // TR
		+1.0f, -1.0f, +0.0f, // BR
		-1.0f, +1.0f, +0.0f, // TL
		-1.0f, -1.0f, +0.0f  // BL
	};
	const std::vector<float> texturePos = {
		1.0f, 1.0f, // TR
		1.0f, 0.0f, // BR
		0.0f, 1.0f, // TL
		0.0f, 0.0f  // BL
	};
	const std::vector<GLuint> indices = {0, 2, 3, 0, 3, 1};
	const static std::vector<std::pair<Texture* const, TextureType>> textures = {
		std::make_pair(texture, TextureType::DIFFUSE)};

	mMesh = new Mesh(vertices, {}, texturePos, indices, textures);
}

void ButtonComponent::draw(const Shader* shader) {
	shader->activate();

	Eigen::Affine3f model = Eigen::Affine3f::Identity();
	Eigen::Vector2f translation = mPadding;
	if (translation.x() < 0) {
		const float x = mOwner->getGame()->getWidth() - translation.x();
		translation.x() = x;
	}
	translation.x() /= mOwner->getGame()->getWidth() * 2 - 1.0f;

	if (translation.y() < 0) {
		const float y = mOwner->getGame()->getHeight() - translation.y();
		translation.y() = y;
	}
	translation.y() /= mOwner->getGame()->getHeight() * 2 - 1.0f;

	model.translate(Eigen::Vector3f(translation.x(), translation.y(), 0.0f));
	model.scale(0.2f);

	shader->set("model", model);

	mMesh->draw(shader);
}

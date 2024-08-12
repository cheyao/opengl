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
		000.0f, 000.0f, -10.0f, // TL
		000.0f, 100.0f, -10.0f, // BL
		100.0f, 000.0f, -10.0f, // TR
		100.0f, 100.0f, -10.0f  // BR
	};
	const std::vector<float> texturePos = {
		1.0f, 1.0f, // TR
		1.0f, 0.0f, // BR
		0.0f, 1.0f, // TL
		0.0f, 0.0f  // BL
	};
	const std::vector<GLuint> indices = {0, 1, 2,  // a
					     1, 2, 3}; // b
	const static std::vector<std::pair<Texture* const, TextureType>> textures = {
		std::make_pair(texture, TextureType::DIFFUSE)};

	mMesh = new Mesh(vertices, {}, texturePos, indices, textures);
}

void ButtonComponent::draw(const Shader* shader) {
	shader->activate();

	Eigen::Affine3f model = Eigen::Affine3f::Identity();
	float x = mPadding.x();
	float y = mPadding.y();

	if (x < 0) {
		x += mOwner->getGame()->getWidth();
	}

	y = -y;
	if (y < 0) {
		y += mOwner->getGame()->getHeight();
	}
	model.translate(Eigen::Vector3f(x, y, 0.0f));

	shader->set("model", model);

	mMesh->draw(shader);
}

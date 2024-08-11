#include "ui/buttonComponent.hpp"

#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"

#include <functional>
#include <vector>

ButtonComponent::ButtonComponent(UIScreen* owner, Texture* const texture, const std::function<void()>& onClick,
				 const Eigen::Vector2f padding)
	: UIComponent(owner), mOnClick(onClick), mPadding(padding) {
	const std::vector<float> vertices = {
		+0.5f, +0.5f, +0.0f, // TR
		+0.5f, -0.5f, +0.0f, // BR
		-0.5f, +0.5f, +0.0f, // TL
		-0.5f, -0.5f, +0.0f  // BL
	};
	const std::vector<float> texturePos = {
		1.0f, 1.0f, // TR
		1.0f, 0.0f, // BR
		0.0f, 1.0f, // TL
		0.0f, 0.0f, // BL
	};
	const std::vector<GLuint> indices = {0, 2, 3, 0, 3, 1};
	const static std::vector<std::pair<Texture* const, TextureType>> textures = {
		std::make_pair(texture, TextureType::DIFFUSE)};

	mMesh = new Mesh(vertices, {}, texturePos, indices, textures);
}

void ButtonComponent::draw([[maybe_unused]] const Shader* shader) {
	shader->activate();

	mMesh->draw(shader);
}

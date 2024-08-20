#include "components/sprite2DComponent.hpp"

#include "actors/actor.hpp"
#include "components/drawComponent.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

Sprite2DComponent::Sprite2DComponent(Actor* owner, Texture* texture, int drawOrder)
	: DrawComponent(owner, drawOrder) {
	const float vertices[] = {
		0.0f * texture->getWidth(), 0.0f * texture->getHeight(), 0.0f, // TL
		0.0f * texture->getWidth(), 1.0f * texture->getHeight(), 0.0f, // BR
		1.0f * texture->getWidth(), 0.0f * texture->getHeight(), 0.0f, // TR
		1.0f * texture->getWidth(), 1.0f * texture->getHeight(), 0.0f  // BL
	};

	constexpr const static float texturePos[] = {
		0.0f, 1.0f, // TR
		0.0f, 0.0f, // BR
		1.0f, 1.0f, // TL
		1.0f, 0.0f  // BL
	};

	const static GLuint indices[] = {2, 1, 0,  // a
					 1, 2, 3}; // b

	mMesh = new Mesh(vertices, {}, texturePos, indices, {{texture, TextureType::DIFFUSE}});
}

void Sprite2DComponent::draw() {
	mMesh->draw(this->getShader());
}

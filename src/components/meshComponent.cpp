#include "components/meshComponent.hpp"

#include "actors/actor.hpp"
#include "components/drawComponent.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <memory>
#include <span>
#include <utility>
#include <vector>

MeshComponent::MeshComponent(Actor* owner, const std::span<const Vertex> vertices,
			     const std::span<const GLuint> indices,
			     const std::vector<std::pair<Texture* const, TextureType>>& textures,
			     int drawOrder)
	: DrawComponent(owner, drawOrder), mMesh(std::make_unique<Mesh>(vertices, indices, textures)) {}

MeshComponent::MeshComponent(Actor* owner, const std::span<const float> positions, const std::span<const float> normals,
			     const std::span<const float> texturePos, const std::span<const GLuint> indices,
			     const std::vector<std::pair<Texture* const, TextureType>>& textures,
			     int drawOrder)
	: DrawComponent(owner, drawOrder),
	  mMesh(std::make_unique<Mesh>(positions, normals, texturePos, indices, textures)) {}

void MeshComponent::draw() {
	if (!getVisible()) {
		return;
	}

	Eigen::Affine3f matrix = Eigen::Affine3f::Identity();
	matrix.translate(mOwner->getPosition());
	matrix.scale(mOwner->getScale());
	matrix.rotate(mOwner->getRotation());
	getShader()->set("model", matrix);

	mMesh->draw(getShader());
}

void MeshComponent::setDrawFunc(
	const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func) {
	mMesh->setDrawFunc(func);
}

void MeshComponent::addUniform(const std::function<void(const Shader*)> func) { mMesh->addUniform(func); }

void MeshComponent::addAttribArray(const GLsizeiptr size, const GLvoid* const data, const std::function<void()>& bind) {
	mMesh->addAttribArray(size, data, bind);
}

void MeshComponent::addAttribArray(const GLuint VBO, const std::function<void()>& bind) {
	mMesh->addAttribArray(VBO, bind);
}

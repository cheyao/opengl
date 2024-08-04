#include "components/meshComponent.hpp"

#include "actors/actor.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <memory>
#include <utility>
#include <vector>

MeshComponent::MeshComponent(Actor* owner, const std::vector<Vertex>& vertices,
							 const std::vector<unsigned int>& indices,
							 const std::vector<std::pair<Texture*, TextureType>>& textures,
							 int drawOrder)
	: DrawComponent(owner, drawOrder), mMesh(std::make_unique<Mesh>(vertices, indices, textures)) {}

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

void MeshComponent::addUniform(const std::function<void(const Shader*)> func) {
	mMesh->addUniform(func);
}

void MeshComponent::addAttribArray(const GLsizeiptr& size, const GLvoid* data,
								   std::function<void()> bind) {
	mMesh->addAttribArray(size, data, bind);
}

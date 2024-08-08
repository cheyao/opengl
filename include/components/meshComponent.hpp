#pragma once

#include "components/drawComponent.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <utility>
#include <vector>

class MeshComponent : public DrawComponent {
      public:
	explicit MeshComponent(
		class Actor* owner, const std::vector<const Vertex>& vertices,
		const std::vector<const GLuint>& indices,
		const std::vector<const std::pair<const class Texture* const, const TextureType>>& textures,
		int drawOrder = 100);
	MeshComponent(MeshComponent&&) = delete;
	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(MeshComponent&&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;
	~MeshComponent() override = default;

	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func);
	void addUniform(const std::function<void(const class Shader*)> func);
	void addAttribArray(const GLsizeiptr& size, const GLvoid* data, std::function<void()> bind, GLuint VBO = -1);

	void draw() override;

      private:
	std::unique_ptr<class Mesh> mMesh;
};

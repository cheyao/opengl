#pragma once

#include "components/drawComponent.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <span>
#include <utility>
#include <vector>

class MeshComponent : public DrawComponent {
      public:
	explicit MeshComponent(
		class Actor* owner, const std::span<const Vertex> vertices, const std::span<const GLuint> indices,
		const std::vector<const std::pair<const class Texture* const, const TextureType>>& textures,
		int drawOrder = 100);
	explicit MeshComponent(class Actor* owner, const std::span<const float> positions,
			       const std::span<const float> normals, const std::span<const float> texturePos,
			       const std::span<const GLuint> indices,
			       const std::vector<const std::pair<const Texture* const, const TextureType>>& textures,
			       int drawOrder = 100);
	MeshComponent(MeshComponent&&) = delete;
	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(MeshComponent&&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;
	~MeshComponent() override = default;

	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func);
	void addUniform(const std::function<void(const class Shader*)> func);

	void addAttribArray(const GLsizeiptr size, const GLvoid* const data, const std::function<void()>& bind);
	void addAttribArray(const GLuint VBO, const std::function<void()>& bind);

	void draw() override;

      private:
	std::unique_ptr<class Mesh> mMesh;
};

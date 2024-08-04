#pragma once

#include "components/drawComponent.hpp"
#include "third_party/glad/glad.h"
#include "opengl/types.hpp"

#include <utility>
#include <vector>

class MeshComponent : public DrawComponent {
  public:
	explicit MeshComponent(class Actor* owner, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
			const std::vector<std::pair<class Texture*, TextureType>>& textures, int drawOrder = 100);
	MeshComponent(MeshComponent&&) = delete;
	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(MeshComponent&&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;
	~MeshComponent() override = default;

	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func);
	void addUniform(const std::function<void(const class Shader*)> func);

	void draw() override;

  private:
	std::unique_ptr<class Mesh> mMesh;
};

#pragma once

#include "components/drawComponent.hpp"
#include "opengl/types.hpp"

#include <utility>
#include <vector>

class MeshComponent : public DrawComponent {
  public:
	explicit MeshComponent(class Actor* owner, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
			const std::vector<std::pair<class Texture*, TextureType>>& textures);
	MeshComponent(MeshComponent&&) = delete;
	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(MeshComponent&&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;
	~MeshComponent();

	void draw() override;

  private:
	std::unique_ptr<class Mesh> mMesh;
};

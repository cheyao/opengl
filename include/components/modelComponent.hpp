#pragma once

#include "components/drawComponent.hpp"

#include <vector>
#include <assimp/material.h>

class ModelComponent : public DrawComponent {
  public:
	explicit ModelComponent(class Actor* owner, const std::string_view& path);
	ModelComponent(ModelComponent&&) = delete;
	ModelComponent(const ModelComponent&) = delete;
	ModelComponent& operator=(ModelComponent&&) = delete;
	ModelComponent& operator=(const ModelComponent&) = delete;
	~ModelComponent();

	void draw() override;

  private:
	void loadNode(struct aiNode* node, const struct aiScene* scene);
	void loadMesh(struct aiMesh* mesh, const struct aiScene* scene);
	std::vector<class Texture*> loadTextures(struct aiMaterial* mat, const aiTextureType type);

	std::vector<class Mesh*> mMeshes;
};

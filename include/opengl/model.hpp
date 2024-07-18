#pragma once

#include <vector>
#include <assimp/material.h>

class Model {
  public:
	explicit Model(const std::string_view& path, class Game* owner);
	Model(Model&&) = delete;
	Model(const Model&) = delete;
	Model& operator=(Model&&) = delete;
	Model& operator=(const Model&) = delete;
	~Model();

	void draw(const class Shader* shader) const;

  private:
	void loadNode(struct aiNode* node, const struct aiScene* scene);
	void loadMesh(struct aiMesh* mesh, const struct aiScene* scene);
	std::vector<class Texture*> loadTextures(struct aiMaterial* mat, const aiTextureType type);

	std::vector<class Mesh*> mMeshes;
	class Game* mOwner;
};

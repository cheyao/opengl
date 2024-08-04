#pragma once

#include "components/drawComponent.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <vector>

class ModelComponent : public DrawComponent {
  public:
	explicit ModelComponent(class Actor* owner, const std::string_view& path);
	ModelComponent(ModelComponent&&) = delete;
	ModelComponent(const ModelComponent&) = delete;
	ModelComponent& operator=(ModelComponent&&) = delete;
	ModelComponent& operator=(const ModelComponent&) = delete;
	~ModelComponent();

	void draw() override;

	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type,
											  const void* indices)>& func);
	void addTexture(std::pair<class Texture*, TextureType> texture);
	void addUniform(const std::function<void(const class Shader* shader)> func);
	void addAttribArray(const GLsizeiptr& size, const GLvoid* data, std::function<void()> bind, GLuint VBO = -1);

  private:
	void loadNode(struct aiNode* node, const struct aiScene* scene);
	void loadMesh(struct aiMesh* mesh, const struct aiScene* scene);
	std::vector<class Texture*> loadTextures(struct aiMaterial* mat, const aiTextureType type);

	std::vector<class Mesh*> mMeshes;
};

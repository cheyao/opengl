#pragma once

#include "components/drawComponent.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <vector>

class ModelComponent : public DrawComponent {
      public:
	explicit ModelComponent(class Actor* owner, const std::string_view& path, const bool useTexture = true);
	ModelComponent(ModelComponent&&) = delete;
	ModelComponent(const ModelComponent&) = delete;
	ModelComponent& operator=(ModelComponent&&) = delete;
	ModelComponent& operator=(const ModelComponent&) = delete;
	~ModelComponent();

	void draw() override;

	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func);
	void addTexture(std::pair<class Texture*, TextureType> texture);
	void addUniform(const std::function<void(class Shader* shader)> func);

	void addAttribArray(const GLsizeiptr size, const GLvoid* const data, const std::function<void()>& bind);
	void addAttribArray(const GLuint VBO, const std::function<void()>& bind);

      private:
	void loadNode(struct aiNode* node, const struct aiScene* scene, const bool useTexture);
	void loadMesh(struct aiMesh* mesh, const struct aiScene* scene, const bool useTexture);
	std::vector<class Texture*> loadTextures(const struct aiMaterial* mat, const aiTextureType type);

	std::vector<class Mesh*> mMeshes;
};

#include "components/modelComponent.hpp"

#include "actors/actor.hpp"
#include "game.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/types.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <stdexcept>
#include <string_view>
#include <utility>

// PERF: Get a manager, no duplicate models
ModelComponent::ModelComponent(Actor* owner, const std::string_view& path, const bool useTexture)
	: DrawComponent(owner) {
	// NOTE: This importer handles memory
	// All data is freed after the destruction of this object
	Assimp::Importer importer;

	const aiScene* scene =
		importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_OptimizeMeshes);
	[[unlikely]] if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to load model with assimp %s: \n%s\n", path.data(),
				importer.GetErrorString());
		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have "
			  "enough memory");

		throw std::runtime_error("ModelComponent.cpp: Failed to read model");
	}

	loadNode(scene->mRootNode, scene, useTexture);

	SDL_Log("Successfully loaded model: %s", path.data());
}

ModelComponent::~ModelComponent() {
	for (const auto& mesh : mMeshes) {
		delete mesh;
	}
}

void ModelComponent::loadNode(aiNode* node, const aiScene* scene, const bool useTexture) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		loadMesh(mesh, scene, useTexture);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		loadNode(node->mChildren[i], scene, useTexture);
	}
}

void ModelComponent::loadMesh(aiMesh* mesh, const aiScene* scene, const bool useTexture) {
	std::vector<unsigned int> indices;
	std::vector<std::pair<Texture* const, TextureType>> textures;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];

		// PERF: Optimize
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.emplace_back(face.mIndices[j]);
		}
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	if (useTexture) {
		// PERF: better
		const std::vector<Texture*> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE);
		const std::vector<Texture*> specularMaps = loadTextures(material, aiTextureType_SPECULAR);
		const std::vector<Texture*> heightMaps = loadTextures(material, aiTextureType_HEIGHT);
		const std::vector<Texture*> ambientMaps = loadTextures(material, aiTextureType_AMBIENT);

		textures.reserve(diffuseMaps.size() + specularMaps.size() + heightMaps.size() + ambientMaps.size());

		for (const auto& texture : diffuseMaps) {
			textures.emplace_back(texture, DIFFUSE);
		}
		for (const auto& texture : specularMaps) {
			textures.emplace_back(texture, SPECULAR);
		}
		for (const auto& texture : heightMaps) {
			textures.emplace_back(texture, HEIGHT);
		}
		for (const auto& texture : ambientMaps) {
			textures.emplace_back(texture, AMBIENT);
		}
	} else {
		textures.clear();
	}

	assert(mesh->mTextureCoords[0] && "Unimplimented");

	mMeshes.emplace_back(new Mesh({&mesh->mVertices[0].x, mesh->mNumVertices * 3},
				      {&mesh->mNormals[0].x, mesh->mNumVertices * 3},
				      {&mesh->mTextureCoords[0][0].x, mesh->mNumVertices * 3 * useTexture}, indices, textures));
}

std::vector<Texture*> ModelComponent::loadTextures(const aiMaterial* mat, const aiTextureType type) {
	std::vector<Texture*> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;

		mat->GetTexture(type, i, &str);

		textures.emplace_back(mOwner->getGame()->getTexture(str.C_Str()));
	}

	return textures;
}

void ModelComponent::draw() {
	if (!getVisible()) {
		return;
	}

	Eigen::Affine3f matrix = Eigen::Affine3f::Identity();
	matrix.translate(mOwner->getPosition());
	matrix.scale(mOwner->getScale());
	matrix.rotate(mOwner->getRotation());
	this->getShader()->set("model", matrix);

	for (const auto& mesh : mMeshes) {
		mesh->draw(this->getShader());
	}
}

void ModelComponent::addTexture(std::pair<class Texture*, TextureType> texture) {
	for (const auto& mesh : mMeshes) {
		mesh->addTexture(texture);
	}
}

void ModelComponent::setDrawFunc(
	const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func) {
	for (const auto& mesh : mMeshes) {
		mesh->setDrawFunc(func);
	}
}

void ModelComponent::addUniform(const std::function<void(const Shader* shader)> func) {
	for (const auto& mesh : mMeshes) {
		mesh->addUniform(func);
	}
}

void ModelComponent::addAttribArray(const GLsizeiptr size, const GLvoid* const data,
				    const std::function<void()>& bind) {
	for (const auto& mesh : mMeshes) {
		mesh->addAttribArray(size, data, bind);
	}
}

void ModelComponent::addAttribArray(const GLuint VBO, const std::function<void()>& bind) {
	for (const auto& mesh : mMeshes) {
		mesh->addAttribArray(VBO, bind);
	}
}

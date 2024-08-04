#include "components/modelComponent.hpp"

#include "actors/actor.hpp"
#include "game.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/types.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdexcept>
#include <string_view>
#include <utility>

ModelComponent::ModelComponent(Actor* owner, const std::string_view& path) : DrawComponent(owner) {
	// NOTE: This importer handles memory
	// All data is freed after this funciton
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path.data(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_OptimizeMeshes);
	[[unlikely]] if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
					 !scene->mRootNode) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to load model with assimp %s: \n%s\n",
						path.data(), importer.GetErrorString());
		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have "
				  "enough memory");

		throw std::runtime_error("ModelComponent.cpp: Failed to read model");
	}

	loadNode(scene->mRootNode, scene);

	SDL_Log("Successfully loaded model: %s", path.data());
}

ModelComponent::~ModelComponent() {
	for (const auto& mesh : mMeshes) {
		delete mesh;
	}

	// delete importer;
}

void ModelComponent::loadNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		loadMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		loadNode(node->mChildren[i], scene);
	}
}

void ModelComponent::loadMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<std::pair<Texture*, TextureType>> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		vertex.position =
			Eigen::Vector3f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal =
			Eigen::Vector3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		if (mesh->mTextureCoords[0]) {
			vertex.texturePos =
				Eigen::Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		} else {
			vertex.texturePos = Eigen::Vector2f(0.0f, 0.0f);
		}

		vertices.emplace_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.emplace_back(face.mIndices[j]);
		}
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	// TODO: better
	const std::vector<Texture*> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE);
	const std::vector<Texture*> specularMaps = loadTextures(material, aiTextureType_SPECULAR);
	const std::vector<Texture*> heightMaps = loadTextures(material, aiTextureType_HEIGHT);
	const std::vector<Texture*> ambientMaps = loadTextures(material, aiTextureType_AMBIENT);

	textures.reserve(diffuseMaps.size() + specularMaps.size() + heightMaps.size() +
					 ambientMaps.size());

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

	mMeshes.emplace_back(new Mesh(vertices, indices, textures));
	// FIXME: Fix mesh
	// mMeshes.emplace_back(new Mesh(reinterpret_cast<float*>(mesh->mVertices),
	// reinterpret_cast<float*>(mesh->mNormals), reinterpret_cast<float*>(mesh->mTextureCoords[0]),
	// mesh->mNumVertices, indices, textures));
}

std::vector<Texture*> ModelComponent::loadTextures(aiMaterial* mat, const aiTextureType type) {
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

void ModelComponent::addAttribArray(const GLsizeiptr& size, const GLvoid* data,
									std::function<void()> bind, GLuint VBO) {
	for (const auto& mesh : mMeshes) {
		mesh->addAttribArray(size, data, bind, VBO);
	}
}

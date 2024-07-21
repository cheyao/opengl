#include "opengl/model.hpp"

#include "game.hpp"
#include "managers/textureManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/texture.hpp"
#include "third_party/Eigen/Core"
#include "tracy/Tracy.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string_view>

Model::Model(const std::string_view& path, Game* owner) : mOwner(owner) {
	ZoneScopedN("Loading model");
	ZoneName(path.data(), path.size());

	// TODO: SDL Importer
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		path.data(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_OptimizeMeshes);
	[[unlikely]] if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
					 !scene->mRootNode) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to load model with assimp %s: \n%s\n",
						path.data(), importer.GetErrorString());
		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have "
				  "enough memory");

		throw 1;
	}

	loadNode(scene->mRootNode, scene);

	SDL_Log("Successfully loaded model: %s", path.data());
}

Model::~Model() {
	for (const auto& mesh : mMeshes) {
		delete mesh;
	}
}

void Model::loadNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		loadMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		loadNode(node->mChildren[i], scene);
	}
}

void Model::loadMesh(aiMesh* mesh, const aiScene* scene) {
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
}

std::vector<Texture*> Model::loadTextures(aiMaterial* mat, const aiTextureType type) {
	std::vector<Texture*> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;

		mat->GetTexture(type, i, &str);

		textures.emplace_back(mOwner->getTexture(str.C_Str()));
	}

	return textures;
}

void Model::draw(const class Shader* shader) const {
	ZoneScopedN("Draw model");

	for (const auto& mesh : mMeshes) {
		mesh->draw(shader);
	}
}

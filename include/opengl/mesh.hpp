#pragma once

#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <utility>
#include <vector>

class Mesh {
  public:
	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
				  const std::vector<std::pair<class Texture*, TextureType>>& textures);
	// explicit Mesh(const float* const positions, const float* const normals);
	[[deprecated("Doesn't work")]] explicit Mesh(float* positions, float* normals, float* texPos, const unsigned int vertCount, const std::vector<unsigned int>& indices, const std::vector<std::pair<class Texture*, TextureType>>& textures);
	Mesh(Mesh&&) = delete;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	void draw(const class Shader* shader) const;

	unsigned int indices() const { return mIndices.size(); }
	// unsigned int vertices() const { return mVertices.size(); }

	void addTexture(std::pair<class Texture*, TextureType> texture) {
		mTextures.emplace_back(texture);
	}

  private:
	void genBuffers();

	GLuint mVBO;
	GLuint mEBO;
	GLuint mVAO;

	// std::vector<struct Vertex> mVertices;
	std::vector<unsigned int> mIndices;
	std::vector<std::pair<class Texture*, TextureType>> mTextures;
};

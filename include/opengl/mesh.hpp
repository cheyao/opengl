#pragma once

#include "opengl/types.hpp"

#include <utility>
#include <vector>

class Mesh {
  public:
	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
		 const std::vector<std::pair<class Texture*, TextureType>>& textures);
	Mesh(Mesh&&) = delete;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	void draw(const class Shader* shader) const;

	unsigned int indices() const { return mIndices.size(); }
	unsigned int vertices() const { return mVertices.size(); }

  private:
	unsigned int mVBO;
	unsigned int mEBO;
	unsigned int mVAO;

	std::vector<struct Vertex> mVertices;
	std::vector<unsigned int> mIndices;
	std::vector<std::pair<class Texture*, TextureType>> mTextures;
};

#pragma once

#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <cstddef>
#include <span>
#include <utility>
#include <vector>

class Mesh {
      public:
	explicit Mesh(std::span<const Vertex> vertices, std::span<const GLuint> indices,
		      const std::vector<std::pair<class Texture* const, TextureType>>& textures = {});
	// Non-owning: no copy, uses subbuffer to construct data in place
	// Spans fit into 2 registers (16 bytes) so no reference
	// Vectors are 24 bytes (from cling) so by ref
	explicit Mesh(std::span<const float> positions, std::span<const float> normals,
		      std::span<const float> texturePos, std::span<const GLuint> indices,
		      const std::vector<std::pair<class Texture* const, TextureType>>& textures = {});

	Mesh(Mesh&&) = delete;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	void draw(class Shader* shader);
	void drawInstanced(GLsizei count);

	std::size_t indices() const { return mIndicesCount; }

	void addTexture(const std::pair<Texture* const, TextureType> texture) { mTextures.emplace_back(texture); }
	void addAttribArray(const GLsizeiptr size, const GLvoid* const data, void (*bind)());
	void addAttribArray(const GLuint VBO, void (*bind)());

      private:
	GLuint mVBO;
	GLuint mEBO;
	GLuint mVAO;

	std::size_t mIndicesCount;
	std::vector<std::pair<class Texture* const, TextureType>> mTextures;

	std::vector<GLuint> mAttribs;
};

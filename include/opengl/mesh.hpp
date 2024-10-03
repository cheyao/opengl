#pragma once

#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <cstddef>
#include <functional>
#include <span>
#include <utility>
#include <vector>

class Mesh {
      public:
	explicit Mesh(const std::span<const Vertex> vertices, const std::span<const GLuint> indices,
		      const std::vector<std::pair<class Texture* const, TextureType>>& textures = {});
	// Non-owning: no copy, uses subbuffer to construct data in place
	// Spans fit into 2 registers (16 bytes) so no reference
	// Vectors are 24 bytes (from cling) so by ref
	explicit Mesh(const std::span<const float> positions, const std::span<const float> normals,
		      const std::span<const float> texturePos, const std::span<const GLuint> indices,
		      const std::vector<std::pair<class Texture* const, TextureType>>& textures = {});

	Mesh(Mesh&&) = delete;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	void draw(class Shader* shader);

	std::size_t indices() const { return mIndicesCount; }

	void addTexture(const std::pair<Texture* const, TextureType> texture) { mTextures.emplace_back(texture); }
	void
	setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)>& func) {
		mDrawFunc = func;
	}
	void addUniform(const std::function<void(class Shader* shader)> func) { mUniformFuncs.emplace_back(func); };
	void addAttribArray(const GLsizeiptr size, const GLvoid* const data, const std::function<void()>& bind);
	void addAttribArray(const GLuint VBO, const std::function<void()>& bind);

      private:
	GLuint mVBO;
	GLuint mEBO;
	GLuint mVAO;

	std::size_t mIndicesCount;
	std::vector<std::pair<class Texture* const, TextureType>> mTextures;

	std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)> mDrawFunc;
	std::vector<std::function<void(Shader* shader)>> mUniformFuncs;

	std::vector<GLuint> mAttribs;
};

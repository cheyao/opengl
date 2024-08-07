#pragma once

#include "opengl/shader.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <functional>
#include <span>
#include <utility>
#include <vector>

class Mesh {
  public:
	// TODO: Also span
	explicit Mesh(
		const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
		const std::vector<std::pair<const class Texture*, const TextureType>>& textures);
	// Non-owning: no copy, uses subbuffer to construct data in place
	// Spans fit into 2 registers (16 bytes) so no reference
	// Vectors are 24 bytes (from cling) so by ref
	explicit Mesh(const std::span<float> positions, const std::span<float> normals,
				  const std::span<float> texturePos, const std::vector<unsigned int>& indices,
				  const std::vector<std::pair<const Texture*, const TextureType>>& textures);

	Mesh(Mesh&&) = delete;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	void draw(const class Shader* shader) const;

	size_t indices() const { return mIndicesCount; }

	void addTexture(std::pair<const class Texture* const, const TextureType> texture) {
		mTextures.emplace_back(texture);
	}
	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type,
											  const void* indices)>& func) {
		mDrawFunc = func;
	}
	void addUniform(const std::function<void(const class Shader* shader)> func) {
		mUniformFuncs.emplace_back(func);
	};
	void addAttribArray(const GLsizeiptr& size, const GLvoid* data, std::function<void()> bind,
						GLuint VBO = -1);

  private:
	GLuint mVBO;
	GLuint mEBO;
	GLuint mVAO;

	size_t mIndicesCount;
	std::vector<std::pair<const class Texture*, const TextureType>> mTextures;

	std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)> mDrawFunc;
	std::vector<std::function<void(const Shader* shader)>> mUniformFuncs;

	std::vector<GLuint> mAttribs;
};

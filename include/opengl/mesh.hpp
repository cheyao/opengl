#pragma once

#include "opengl/shader.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <any>
#include <functional>
#include <utility>
#include <vector>

class Mesh {
  public:
	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
				  const std::vector<std::pair<class Texture*, TextureType>>& textures);
	// TODO:
	explicit Mesh(const std::vector<float> positions);
	//, float* normals, float* texPos,
	// const unsigned int vertCount, const std::vector<unsigned int>& indices, const
	// std::vector<std::pair<class Texture*, TextureType>>& textures);

	Mesh(Mesh&&) = delete;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	~Mesh();

	void draw(const class Shader* shader) const;

	size_t indices() const { return mIndicesSize; }
	// unsigned int vertices() const { return mVertices.size(); }

	void addTexture(std::pair<class Texture*, TextureType> texture) {
		mTextures.emplace_back(texture);
	}
	void setDrawFunc(const std::function<void(GLenum mode, GLsizei count, GLenum type,
											  const void* indices)>& func) {
		mDrawFunc = func;
	}
	void addUniform(const std::function<void(const class Shader* shader)> func) {
		mUniformFuncs.emplace_back(func);
	};
	// TODO: Less args
	/*
	   void addAttribArray(const GLuint& index, const GLint& count, const GLsizei& stride,
						   const GLsizeiptr& size, const GLvoid* data,
						   const GLuint& divisor = 0, const GLenum& type = GL_FLOAT);
	*/
	void addAttribArray(const GLsizeiptr& size, const GLvoid* data, std::function<void()> bind, GLuint VBO = -1);

  private:
	GLuint mVBO;
	GLuint mEBO;
	GLuint mVAO;

	// std::vector<struct Vertex> mVertices;
	size_t mIndicesSize;
	std::vector<std::pair<class Texture*, TextureType>> mTextures;

	std::function<void(GLenum mode, GLsizei count, GLenum type, const void* indices)> mDrawFunc;
	std::vector<std::function<void(const Shader* shader)>> mUniformFuncs;

	std::vector<GLuint> mAttribs;
};

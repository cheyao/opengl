#include "opengl/mesh.hpp"

#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>
#include <version>

// TODO: Accept non-Vertex data
Mesh::Mesh(const std::span<const Vertex> vertices, const std::span<const GLuint> indices,
	   const std::vector<std::pair<Texture* const, TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mIndicesCount(indices.size()), mTextures(textures) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindVertexArray(mVAO);

	// Bind all the vertex data
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// Set the attrib pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			      reinterpret_cast<GLvoid*>(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			      reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			      reinterpret_cast<GLvoid*>(offsetof(Vertex, texturePos)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndicesCount * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Mesh::Mesh(const std::span<const float> positions, const std::span<const float> normals,
	   const std::span<const float> texturePos, const std::span<const GLuint> indices,
	   const std::vector<std::pair<Texture* const, TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mIndicesCount(indices.size()), mTextures(textures) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, (positions.size() + normals.size() + texturePos.size()) * sizeof(float), nullptr,
		     GL_STATIC_DRAW);

	std::size_t offset = 0;

	// TODO: Prettier
	glBindVertexArray(mVAO);
	glEnableVertexAttribArray(0);
	// Enable no matter what
	// https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices#always_enable_vertex_attrib_0_as_an_array
	if (!positions.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, positions.size() * sizeof(float), positions.data());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<GLvoid*>(offset));

		offset += positions.size() * sizeof(float);
	} else {
		SDL_Log("\033[33mMesh.cpp: Positions empty, ignored\033[0m");
	}

	if (!normals.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, normals.size() * sizeof(float), normals.data());
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(1);

		offset += normals.size() * sizeof(float);
	} else {
		SDL_Log("\033[33mMesh.cpp: Normals empty, ignored\033[0m");
	}

	if (!texturePos.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, texturePos.size() * sizeof(float), texturePos.data());
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(2);

		offset += texturePos.size() * sizeof(float);
	} else {
		SDL_Log("\033[33mMesh.cpp: Texture pos empty, ignored\033[0m");
	}

	SDL_assert((positions.size() + normals.size() + texturePos.size()) * sizeof(float) == offset && "Missing data");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndicesCount * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);

	// Do not delete double buffers
#ifdef __cpp_lib_ranges
	const auto& u = std::ranges::unique(mAttribs);
	glDeleteBuffers(mAttribs.size() - u.size(), mAttribs.data());
#else
	const auto& u = std::unique(mAttribs.begin(), mAttribs.end());
	mAttribs.erase(u, mAttribs.end());
	glDeleteBuffers(mAttribs.size(), mAttribs.data());
#endif
}

/*
 * NOTE:
 * After calling `glBufferData` the data is already copied and need not to be conserved.
 * Source:
 * https://gamedev.stackexchange.com/questions/187420/opengl-what-to-do-after-running-glbufferdata
 */

/*
 * FIXME: One VBO can only be safely passed to one mesh
 * or else if one mesh get destroed, the VBO will be freed
 * and the other mesh would be accessing an invalid VBO
 *
 * Possible fix: Use std::shared_ptr
 * Cons: This will cause some performance overhead
 *
 * Possible fix: Make a renderer manage all VBO, free on end
 * of application
 * Cons: Memory overhead
 *
 * Possible fix: Make a VBO manager, and count references
 * Cons: Hard to implement without causing more bugs
 */

/*
 * bind: function to bind vertexes
 *
 * Example:
 * glEnableVertexAttribArray(index);
 * glVertexAttribPointer(index, count, type, GL_FALSE, stride, reinterpret_cast<GLvoid*>(0));
 * glVertexAttribDivisor(index, divisor);
 */
void Mesh::addAttribArray(const GLsizeiptr size, const GLvoid* const data, void (*bind)()) {
	GLuint VBO;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	addAttribArray(VBO, bind);
}

void Mesh::addAttribArray(const GLuint VBO, void (*bind)()) {
	SDL_assert(glIsBuffer(VBO));

	glBindVertexArray(mVAO); // Save it in vertex array

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Be sure our VAO contains the VBO

	bind();

	mAttribs.emplace_back(VBO);
}

void Mesh::draw(Shader*) {
	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, mIndicesCount, GL_UNSIGNED_INT, nullptr);
}

void Mesh::drawInstanced(const GLsizei count) {
	glBindVertexArray(mVAO);
	glDrawElementsInstanced(GL_TRIANGLES, mIndicesCount, GL_UNSIGNED_INT, nullptr, count);
}

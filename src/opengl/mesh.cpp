#include "opengl/mesh.hpp"

#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <version>

// TODO: Accept non-Vertex data
Mesh::Mesh(const std::span<const Vertex> vertices, const std::span<const GLuint> indices,
	   const std::vector<std::pair<Texture* const, TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mIndicesCount(indices.size()), mTextures(textures), mDrawFunc(glDrawElements) {
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
	: mVBO(0), mEBO(0), mVAO(0), mIndicesCount(indices.size()), mTextures(textures), mDrawFunc(glDrawElements) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);
	// SDL_Log("%zu", mIndicesCount);

	glGenVertexArrays(1, &mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, (positions.size() + normals.size() + texturePos.size()) * sizeof(float), nullptr,
		     GL_STATIC_DRAW);

	static_assert(std::is_same_v<decltype(positions[0]), decltype(normals[0])>);
	static_assert(std::is_same_v<decltype(texturePos[0]), decltype(normals[0])>);
	static_assert(std::is_same_v<decltype(texturePos[0]), const float&>);

	std::size_t offset = 0;

	// TODO: Non-hardcoded attrib pointer strides
	// TODO: Prettier
	glBindVertexArray(mVAO);
	if (!positions.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, positions.size() * sizeof(float), positions.data());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(0);

		offset += positions.size() * sizeof(float);
	} else {
		SDL_Log("\033[33mMesh.cpp: Normals empty, ignored\033[0m");
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

	glBindVertexArray(0);
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

// NOTE: sizeof(std::function<>) == 48

/*
 * bind: function to bind vertexes
 *
 * Example:
 * glEnableVertexAttribArray(index);
 * glVertexAttribPointer(index, count, type, GL_FALSE, stride, reinterpret_cast<GLvoid*>(0));
 * glVertexAttribDivisor(index, divisor);
 */
void Mesh::addAttribArray(const GLsizeiptr size, const GLvoid* const data, const std::function<void()>& bind) {
	GLuint VBO;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	addAttribArray(VBO, bind);
}

void Mesh::addAttribArray(const GLuint VBO, const std::function<void()>& bind) {
	SDL_assert(glIsBuffer(VBO));

	glBindVertexArray(mVAO); // Save it in vertex array

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Be sure our VAO contains the VBO

	bind();

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Maybe before attrib divisor?

	mAttribs.emplace_back(VBO);
}

void Mesh::draw(Shader* shader) {
	unsigned int diffuse = 0;
	unsigned int specular = 0;
	unsigned int height = 0;
	unsigned int ambient = 0;

	for (unsigned int i = 0; i < mTextures.size(); i++) {
		std::string name;
		name.reserve(20);

		// += to avoid extra copy that + returns
		switch (mTextures[i].second) {
			case DIFFUSE:
				name = "texture_diffuse";
				name += std::to_string(diffuse++);
				break;
			case SPECULAR:
				name = "texture_specular";
				name += std::to_string(specular++);
				break;
			case HEIGHT:
				name = "texture_height";
				name += std::to_string(height++);
				break;
			case AMBIENT:
				name = "texture_ambient";
				name += std::to_string(ambient++);
				break;
		}

		shader->set(name, static_cast<GLint>(i));

		mTextures[i].first->activate(i);
	}

	for (const std::function<void(Shader*)>& func : mUniformFuncs) {
		func(shader);
	}

	glBindVertexArray(mVAO);
	mDrawFunc(GL_TRIANGLES, mIndicesCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

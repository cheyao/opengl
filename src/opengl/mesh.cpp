#include "opengl/mesh.hpp"

#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <SDL3/SDL.h>
#include <vector>

// FIXME: Use something safer then `reinterpret_cast`

// TODO: Accept non-Vertex data
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
		   const std::vector<std::pair<const Texture*, const TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mIndicesCount(indices.size()), mTextures(textures),
	  mDrawFunc(glDrawElements) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindVertexArray(mVAO);

	// Bind all the vertex data
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(),
				 GL_STATIC_DRAW);

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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(),
				 GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Mesh::Mesh(const std::span<float> positions, const std::span<float> normals,
		   const std::span<float> texturePos, const std::vector<unsigned int>& indices,
		   const std::vector<std::pair<const Texture*, const TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mIndicesCount(indices.size()), mTextures(textures),
	  mDrawFunc(glDrawElements) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	// TODO: Non-hardcoded attrib pointer strides

	assert(!positions.empty());
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size(), positions.data());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
							  reinterpret_cast<GLvoid*>(0));
		glEnableVertexAttribArray(0);
	}

	[[unlikely]] if (!normals.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, positions.size(), normals.size(), normals.data());
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
							  reinterpret_cast<GLvoid*>(positions.size()));
		glEnableVertexAttribArray(1);
	} else {
		SDL_Log("Mesh.cpp: Normals empty, ignored");
	}

	if (!texturePos.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, positions.size() + normals.size(), texturePos.size(),
						texturePos.data());
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
							  reinterpret_cast<GLvoid*>(positions.size() + normals.size()));
		glEnableVertexAttribArray(2);
	} else {
		SDL_Log("Mesh.cpp: Texture pos empty, ignored");
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(),
				 GL_STATIC_DRAW);

	glBindVertexArray(mVAO);

	glBindVertexArray(0);
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
void Mesh::addAttribArray(const GLsizeiptr& size, const GLvoid* data, std::function<void()> bind,
						  GLuint VBO) {

	if (VBO == static_cast<GLuint>(-1)) {
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	glBindVertexArray(mVAO); // Save it in vertex array

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Be sure our VAO contains the VBO

	bind();

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Maybe before attrib divisor?

	mAttribs.emplace_back(VBO);
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);

	// Do not delete double buffers
	const auto& u = std::ranges::unique(mAttribs);
	for (auto it = mAttribs.begin(); it != u.begin(); ++it) {
		glDeleteBuffers(1, &(*it));
	}
}

void Mesh::draw(const Shader* shader) const {
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

	for (const std::function<void(const Shader*)>& func : mUniformFuncs) {
		func(shader);
	}

	glBindVertexArray(mVAO);
	mDrawFunc(GL_TRIANGLES, mIndicesCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

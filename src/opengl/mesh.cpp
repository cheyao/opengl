#include "opengl/mesh.hpp"

#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
		   const std::vector<std::pair<Texture*, TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mVertices(vertices), mIndices(indices), mTextures(textures), mDrawFunc(glDrawElements) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(),
				 GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(mIndices[0]), indices.data(),
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  reinterpret_cast<GLvoid*>(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  reinterpret_cast<GLvoid*>(offsetof(Vertex, texturePos)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

/*
// PERF: Maybe indices pointer?
//
Mesh::Mesh(float* positions, float* normals, float* texPos,
		   const unsigned int vertCount, const std::vector<unsigned int>& indices,
		   const std::vector<std::pair<class Texture*, TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mIndices(indices), mTextures(textures) {
	genBuffers();
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferSubData(GL_ARRAY_BUFFER, vertCount * 0 * sizeof(float), vertCount * 3 * sizeof(float), positions);
	glBufferSubData(GL_ARRAY_BUFFER, vertCount * 3 * sizeof(float), vertCount * 3 * sizeof(float), normals);
	glBufferSubData(GL_ARRAY_BUFFER, vertCount * 6 * sizeof(float), vertCount * 2 * sizeof(float), texPos);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(mIndices[0]), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(vertCount * 0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	if (normals != nullptr) {
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(vertCount * 3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	if (texPos != nullptr) {
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<GLvoid*>(vertCount * 6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	glBindVertexArray(0);
}
*/

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);
}

void Mesh::draw(const Shader* shader) const {
	unsigned int diffuse = 0;
	unsigned int specular = 0;
	unsigned int height = 0;
	unsigned int ambient = 0;

	for (unsigned int i = 0; i < mTextures.size(); i++) {
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name;

		switch (mTextures[i].second) {
			case DIFFUSE:
				number = std::to_string(diffuse++);
				name = "texture_diffuse";
				break;
			case SPECULAR:
				number = std::to_string(specular++);
				name = "texture_specular";
				break;
			case HEIGHT:
				number = std::to_string(height++);
				name = "texture_height";
				break;
			case AMBIENT:
				number = std::to_string(ambient++);
				name = "texture_ambient";
				break;
		}

		shader->set(name + number, static_cast<GLint>(i));

		mTextures[i].first->activate(i);
		// TODO: Std function????
		// https://stackoverflow.com/questions/10022789/stdfunction-with-non-static-member-functions
	}

	glBindVertexArray(mVAO);
	mDrawFunc(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

#include "opengl/mesh.hpp"

#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "third_party/glad/glad.h"

#include <string>
#include <utility>
#include <vector>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
		   const std::vector<std::pair<Texture*, TextureType>>& textures)
	: mVBO(0), mEBO(0), mVAO(0), mVertices(vertices), mIndices(indices), mTextures(textures) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	// Save the conf to VAO
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(mVertices[0]), mVertices.data(),
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

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);
}

#include <SDL3/SDL.h>
#include <iostream>
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

		shader->set(name + number, i);

		mTextures[i].first->activate(i);
		// TODO: Std function????
		// https://stackoverflow.com/questions/10022789/stdfunction-with-non-static-member-functions
	}

	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

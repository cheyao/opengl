#include "vertexArray.hpp"

#include <glad/glad.h>

VertexArray::VertexArray(const float* vertices, unsigned int numVertices,
						 const unsigned int* indices, unsigned int numIndices)
	: mVertices(numVertices), mIndices(numIndices) {
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	// Save the conf to VAO
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(vertices[0]), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(indices[0]), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(vertices[0]), static_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(vertices[0]), reinterpret_cast<GLvoid*>(2 * sizeof(vertices[0])));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(vertices[0]), reinterpret_cast<GLvoid*>(5 * sizeof(vertices[0])));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);
}

void VertexArray::activate() { glBindVertexArray(mVAO); }

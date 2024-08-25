#include "opengl/ubo.hpp"

#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"

UBO::UBO(GLsizeiptr size) : mSize(size) {
	glGenBuffers(1, &mUBO);

	glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW); // allocate 152 bytes of memory
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UBO::~UBO() { glDeleteBuffers(1, &mUBO); }

void UBO::set(GLintptr offset, Eigen::Affine3f matrix) const {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(matrix), matrix.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO::bind(GLuint index) const {
	glBindBufferRange(GL_UNIFORM_BUFFER, index, mUBO, 0, mSize);
}

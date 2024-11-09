#pragma once

#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"

class UBO {
      public:
	explicit UBO(GLsizeiptr size);
	UBO(UBO&&) = delete;
	UBO(const UBO&) = delete;
	UBO& operator=(UBO&&) = delete;
	UBO& operator=(const UBO&) = delete;
	~UBO();

	void set(GLintptr name, const Eigen::Affine3f& matrix) const;
	void bind(GLuint index) const;

      private:
	const GLsizeiptr mSize;
	GLuint mUBO;
};

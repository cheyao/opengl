#pragma once

#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"

class UBO {
      public:
	explicit UBO(GLsizeiptr size);
	UBO(UBO&&);
	UBO(const UBO&);
	UBO& operator=(UBO&&);
	UBO& operator=(const UBO&);
	~UBO();

	void set(GLintptr name, Eigen::Affine3f matrix) const;
	void bind(GLuint index) const;

      private:
	GLuint mUBO;
	GLsizeiptr mSize;
};

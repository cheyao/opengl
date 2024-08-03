#pragma once

#include "third_party/glad/glad.h"
#include "third_party/Eigen/Geometry"

#include <string_view>

class UBO {
  public:
	explicit UBO(GLsizeiptr size);
	UBO(UBO&&);
	UBO(const UBO&);
	UBO& operator=(UBO&&);
	UBO& operator=(const UBO&);
	~UBO();

	// TODO: More setters
	void set(GLintptr name, Eigen::Affine3f matrix);
	void bind(GLuint index);

  private:
	GLuint mUBO;
	GLsizeiptr mSize;
};

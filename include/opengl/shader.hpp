#pragma once

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"

#include <string_view>

class Shader {
  public:
	// explicit Shader(const std::string_view& vertName, const std::string_view& fragName);
	explicit Shader(const std::string_view& vertName, const std::string_view& fragName, const std::string_view geomName = "");
	Shader(Shader&&) = delete;
	Shader(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	~Shader();

	void activate() const;

	// set uniform
	void set(const std::string_view& name, const GLboolean& val) const;
	void set(const std::string_view& name, const GLint& val) const;
	void set(const std::string_view& name, const GLuint& val) const;
	void set(const std::string_view& name, const GLfloat& val) const;
	void set(const std::string_view& name, const GLdouble& val) const;
	void set(const std::string_view& name, const GLfloat& val, const GLfloat& val2) const;
	void set(const std::string_view& name, const GLfloat& val, const GLfloat& val2,
			 const GLfloat& val3) const;
	void set(const std::string_view& name, const Eigen::Vector2f& val) const;
	void set(const std::string_view& name, const Eigen::Vector3f& val) const;
	void set(const std::string_view& name, const Eigen::Vector3f& val, const GLfloat& val2) const;
	void set(const std::string_view& name, const Eigen::Vector4f& val) const;
	// Note: Eigen uses column major storage as default, so no transpose
	void set(const std::string_view& name, const Eigen::Affine3f& mat,
			 const GLboolean& transpose = GL_FALSE) const;

	void bind(std::string_view name, GLuint index);

  private:
	static void compile(const std::string_view& fileName, const GLenum& type, GLuint& out);

	void setUniform(const std::string_view& name, std::function<void(GLint)> toCall) const;

	GLuint mShaderProgram;
};

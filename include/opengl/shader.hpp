#pragma once

#include <third_party/Eigen/Dense>
#include <third_party/glad/glad.h>

#include <string>

class Shader {
  public:
	explicit Shader(const std::string& vertName, const std::string& fragName);
	Shader(Shader&&) = delete;
	Shader(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	~Shader();

	void activate() const;

	// set uniform
	void set(const std::string& name, const bool& val) const;
	void set(const std::string& name, const int& val) const;
	void set(const std::string& name, const float& val) const;
	void set(const std::string& name, const float& val, const float& val2) const;
	void set(const std::string& name, const float& val, const float& val2, const float& val3) const;
	void set(const std::string& name, const Eigen::Vector3f& val) const;
	void set(const std::string& name, const Eigen::Vector3f& val, const float& val2) const;
	void set(const std::string& name, const Eigen::Vector4f& val) const;
	// Note: Eigen uses column major storage as default, so no transpose
	void set(const std::string& name, const Eigen::Affine3f& mat,
			 const GLboolean& transpose = GL_FALSE) const;

  private:
	static void compile(const std::string& fileName, const GLenum& type, unsigned int& out);

	int getUniform(const std::string& name) const;

	unsigned int mVertexShader;
	unsigned int mFragmentShader;
	unsigned int mShaderProgram;
};

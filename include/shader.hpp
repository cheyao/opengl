#pragma once

#include <Eigen/Dense>

#include <glad/glad.h>

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
	void set(const std::string& name, bool val) const;
	void set(const std::string& name, int val) const;
	void set(const std::string& name, float val) const;
	void set(const std::string& name, float val, float val2) const;
	// Note: Eigen uses column major storage as default, so no transpose
	void set(const std::string& name, Eigen::Affine3f mat, GLboolean transpose = GL_FALSE) const;

  private:
	static void compile(const std::string& fileName, const GLenum& type, unsigned int& out);

	int getUniform(const std::string& name) const;

	unsigned int mVertexShader;
	unsigned int mFragmentShader;
	unsigned int mShaderProgram;
};

#pragma once

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

	void activate();

  private:
	void compile(const std::string& fileName, GLenum type, unsigned int& out);

	unsigned int mVertexShader;
	unsigned int mFragmentShader;
	unsigned int mShaderProgram;
};

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

	// set uniform
	void set(const std::string& name, bool val);
	void set(const std::string& name, int val);
	void set(const std::string& name, float val);
	void set(const std::string& name, float val, float val2);

  private:
	void compile(const std::string& fileName, const GLenum& type, unsigned int& out);

	int getUniform(const std::string& name);

	unsigned int mVertexShader;
	unsigned int mFragmentShader;
	unsigned int mShaderProgram;
};

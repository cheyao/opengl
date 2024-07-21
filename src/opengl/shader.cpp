#include "opengl/shader.hpp"
#include "utils.hpp"

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <string_view>

Shader::Shader(const std::string_view& vertName, const std::string_view& fragName)
	: mShaderProgram(glCreateProgram()) {
	unsigned int mVertexShader = 0;
	unsigned int mFragmentShader = 0;

	compile(vertName, GL_VERTEX_SHADER, mVertexShader);
	compile(fragName, GL_FRAGMENT_SHADER, mFragmentShader);

	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragmentShader);
	glLinkProgram(mShaderProgram);

	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);

	int success = 0;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
	[[unlikely]] if (success == 0) {
		int len = 0;
		glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];

		glGetProgramInfoLog(mShaderProgram, 512, nullptr, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to link shader: \n%s\n", log);
		ERROR_BOX("Failed to link shader");

		delete[] log;

		throw 1;
	}
}

Shader::~Shader() {
#ifndef ADDRESS
	glDeleteProgram(mShaderProgram);
#endif
}

void Shader::activate() const { glUseProgram(mShaderProgram); }

int Shader::getUniform(const std::string_view& name) const {
	int location = glGetUniformLocation(mShaderProgram, name.data());

#ifdef DEBUG
	static std::unordered_map<std::string, bool> errored;

	[[unlikely]] if (location == -1 && !errored.contains(std::string(name))) {
		SDL_Log("Failed find uniform location: %s\n", name.data());
		errored[std::string(name)] = true;

		// ERROR_BOX("Failed to find uniform location");
		// throw 1;
	}
#endif

	return location;
}

void Shader::set(const std::string_view& name, const bool& val) const {
	glUniform1i(getUniform(name), static_cast<int>(val));
}

void Shader::set(const std::string_view& name, const int& val) const {
	glUniform1i(getUniform(name), val);
}

void Shader::set(const std::string_view& name, const unsigned int& val) const {
	glUniform1ui(getUniform(name), val);
}

void Shader::set(const std::string_view& name, const float& val) const {
	glUniform1f(getUniform(name), val);
}

void Shader::set(const std::string_view& name, const double& val) const {
	glUniform1f(getUniform(name), static_cast<float>(val));
}

void Shader::set(const std::string_view& name, const float& val, const float& val2) const {
	glUniform2f(getUniform(name), val, val2);
}

void Shader::set(const std::string_view& name, const float& val, const float& val2,
				 const float& val3) const {
	glUniform3f(getUniform(name), val, val2, val3);
}

void Shader::set(const std::string_view& name, const Eigen::Vector3f& val) const {
	glUniform3f(getUniform(name), val.x(), val.y(), val.z());
}

void Shader::set(const std::string_view& name, const Eigen::Vector3f& val,
				 const float& val2) const {
	glUniform4f(getUniform(name), val.x(), val.y(), val.z(), val2);
}

void Shader::set(const std::string_view& name, const Eigen::Vector4f& val) const {
	glUniform4f(getUniform(name), val.x(), val.y(), val.z(), val.w());
}

void Shader::set(const std::string_view& name, const Eigen::Affine3f& mat,
				 const GLboolean& transpose) const {
	glUniformMatrix4fv(getUniform(name), 1, transpose, mat.data());
}

void Shader::compile(const std::string_view& fileName, const GLenum& type, unsigned int& out) {
	char* shaderSource = static_cast<char*>(SDL_LoadFile(fileName.data(), nullptr));
#ifdef GLES
	// #version 400 core
	// to
	// #version 300 es
	shaderSource[9] = '3';
	shaderSource[13] = 'e';
	shaderSource[14] = 's';
	shaderSource[15] = ' ';
	shaderSource[16] = ' ';
#endif

	[[unlikely]] if (shaderSource == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to read shader shource: %s\n",
						fileName.data());
		ERROR_BOX("Failed to read assets");

		throw 1;
	}
	SDL_Log("Loading %s", fileName.data());

	out = glCreateShader(type);
	glShaderSource(out, 1, &shaderSource, nullptr);
	glCompileShader(out);

	SDL_free(shaderSource);

	// Error checking
	// FIXME: Some wierd error handleing
	int success = 0;
	glGetShaderiv(out, GL_COMPILE_STATUS, &success);
	[[unlikely]] if (success == 0) {
		int len = 0;
		glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];

		glGetShaderInfoLog(out, len * sizeof(GLchar), nullptr, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile shader %s: \n%s\n",
						fileName.data(), log);
		ERROR_BOX("Failed to compile shader");

		delete[] log;

		throw 1;
	}
}

#include "opengl/shader.hpp"
#include "utils.hpp"

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

/*
Shader::Shader(const std::string_view& vertName, const std::string_view& fragName)
	: mShaderProgram(glCreateProgram()) {
	GLuint mVertexShader = 0;
	GLuint mFragmentShader = 0;

	compile(vertName, GL_VERTEX_SHADER, mVertexShader);
	compile(fragName, GL_FRAGMENT_SHADER, mFragmentShader);

	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragmentShader);
	glLinkProgram(mShaderProgram);

	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);

	GLint success = 0;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
	[[unlikely]] if (success == 0 || !glIsProgram(mShaderProgram)) {
		GLint len = 0;
		glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];

		glGetProgramInfoLog(mShaderProgram, 512, nullptr, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to link shader: \n%s\n", log);
		ERROR_BOX("Failed to link shader");

		delete[] log;

		throw std::runtime_error("Shader.cpp: Failed to link shader");
	}
}
*/

Shader::Shader(const std::string_view& vertName, const std::string_view& fragName,
			   const std::string_view geomName)
	: mShaderProgram(glCreateProgram()) {
	GLuint mVertexShader = 0;
	GLuint mFragmentShader = 0;
	GLuint mGeometryShader = 0;

	compile(vertName, GL_VERTEX_SHADER, mVertexShader);
	compile(fragName, GL_FRAGMENT_SHADER, mFragmentShader);
	if (!geomName.empty()) {
		compile(geomName, GL_GEOMETRY_SHADER, mGeometryShader);
	}

	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragmentShader);
	if (!geomName.empty()) {
		glAttachShader(mShaderProgram, mGeometryShader);
	}
	glLinkProgram(mShaderProgram);

	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);
	if (!geomName.empty()) {
		glDeleteShader(mGeometryShader);
	}

	GLint success = 0;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
	[[unlikely]] if (success == 0 || !glIsProgram(mShaderProgram)) {
		GLint len = 0;
		glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];

		glGetProgramInfoLog(mShaderProgram, 512, nullptr, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to link shader: \n%s\n", log);
		ERROR_BOX("Failed to link shader");

		delete[] log;

		throw std::runtime_error("Shader.cpp: Failed to link shader");
	}
}

Shader::~Shader() {
#ifndef ADDRESS
	glDeleteProgram(mShaderProgram);
#endif
}

void Shader::activate() const { glUseProgram(mShaderProgram); }

void Shader::setUniform(const std::string_view& name, std::function<void(GLint)> toCall) const {
	// Safety checks
	assert(!name.contains(' '));
	assert(!(name[0] == 'g' && name[1] == 'l' && name[2] == '_'));

	int location = glGetUniformLocation(mShaderProgram, name.data());

#ifdef DEBUG
	static std::unordered_map<std::string, bool> errored;

	[[unlikely]] if (location == -1) {
		if (!errored.contains(std::string(name))) {
			SDL_Log("Failed find uniform location \"%s\"\n", name.data());

			errored[std::string(name)] = true;
		}
		// TODO: Error?

		return;
	}
#else
	[[unlikely]] if (location == -1) { return; }
#endif

	toCall(location);
}

void Shader::set(const std::string_view& name, const GLboolean& val) const {
	setUniform(name, std::bind(glUniform1i, std::placeholders::_1, static_cast<GLint>(val)));
}
void Shader::set(const std::string_view& name, const GLint& val) const {
	setUniform(name, std::bind(glUniform1i, std::placeholders::_1, val));
}
void Shader::set(const std::string_view& name, const GLuint& val) const {
	setUniform(name, std::bind(glUniform1ui, std::placeholders::_1, val));
}
void Shader::set(const std::string_view& name, const GLfloat& val) const {
	setUniform(name, std::bind(glUniform1f, std::placeholders::_1, val));
}
void Shader::set(const std::string_view& name, const GLdouble& val) const {
	setUniform(name, std::bind(glUniform1f, std::placeholders::_1, static_cast<GLfloat>(val)));
}
void Shader::set(const std::string_view& name, const GLfloat& val, const GLfloat& val2) const {
	setUniform(name, std::bind(glUniform2f, std::placeholders::_1, val, val2));
}
void Shader::set(const std::string_view& name, const GLfloat& val, const GLfloat& val2,
				 const GLfloat& val3) const {
	setUniform(name, std::bind(glUniform3f, std::placeholders::_1, val, val2, val3));
}
void Shader::set(const std::string_view& name, const Eigen::Vector3f& val) const {
	setUniform(name, std::bind(glUniform3f, std::placeholders::_1, val.x(), val.y(), val.z()));
}
void Shader::set(const std::string_view& name, const Eigen::Vector3f& val,
				 const GLfloat& val2) const {
	setUniform(name,
			   std::bind(glUniform4f, std::placeholders::_1, val.x(), val.y(), val.z(), val2));
}
void Shader::set(const std::string_view& name, const Eigen::Vector4f& val) const {
	setUniform(name,
			   std::bind(glUniform4f, std::placeholders::_1, val.x(), val.y(), val.z(), val.w()));
}
void Shader::set(const std::string_view& name, const Eigen::Affine3f& mat,
				 const GLboolean& transpose) const {
	setUniform(name,
			   std::bind(glUniformMatrix4fv, std::placeholders::_1, 1, transpose, mat.data()));
}

void Shader::bind(std::string_view name, GLuint index) {
	GLuint blockIndex = glGetUniformBlockIndex(mShaderProgram, name.data());

	if (blockIndex == GL_INVALID_INDEX) {
		// TODO: Warn once
		SDL_Log("Invalid block: %s", name.data());

		return;
	}

	glUniformBlockBinding(mShaderProgram, blockIndex, index);
}

void Shader::compile(const std::string_view& fileName, const GLenum& type, GLuint& out) {
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
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to read shader shource %s: %s\n",
						fileName.data(), SDL_GetError());

#ifndef DEBUG
		ERROR_BOX("Failed to read assets");
#endif

		throw std::runtime_error("shader.cpp: Failed to read shader source");
	}
	SDL_Log("Loading %s", fileName.data());

	out = glCreateShader(type);
	glShaderSource(out, 1, &shaderSource, nullptr);
	glCompileShader(out);

	SDL_free(shaderSource);

	// Error checking
	GLint success = 0;
	glGetShaderiv(out, GL_COMPILE_STATUS, &success);
	[[unlikely]] if (success == 0) {
		GLint len = 0;
		glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];

		glGetShaderInfoLog(out, len * sizeof(GLchar), nullptr, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile shader %s: \n%s\n",
						fileName.data(), log);

		ERROR_BOX("Failed to compile shader");

		delete[] log;

		throw std::runtime_error("shader.cpp: Failed to compile shader");
	}
}

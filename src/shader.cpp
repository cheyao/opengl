#include "shader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <cassert>

#include "common.hpp"

Shader::Shader(const std::string& vertName, const std::string& fragName) {
	compile(vertName, GL_VERTEX_SHADER, mVertexShader);
	compile(fragName, GL_FRAGMENT_SHADER, mFragmentShader);

	mShaderProgram = glCreateProgram();
	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragmentShader);
	glLinkProgram(mShaderProgram);

	int success;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		int len = 0;
		glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);
		std::string log;
		log.resize(len);

		glGetProgramInfoLog(mShaderProgram, 512, NULL, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to link shader: %s\n", log.c_str());
		ERROR_BOX("Failed to link shader");

		throw 1;
	}
}

Shader::~Shader() {
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);
	glDeleteProgram(mShaderProgram);
}

void Shader::activate() {
	glUseProgram(mShaderProgram);

	/*
	glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
	*/
}

int Shader::getUniform(const std::string& name) {
	int location = glGetUniformLocation(mShaderProgram, name.c_str());

	if (location == -1) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed find uniform location: %s\n", name.c_str());
		ERROR_BOX("Failed to find uniform location");
		throw 1;
	}

	return location;
}

void Shader::set(const std::string& name, bool val) {
	glUniform1i(getUniform(name), static_cast<int>(val)); 
}

void Shader::set(const std::string& name, int val) {
	glUniform1i(getUniform(name), val); 
}

void Shader::set(const std::string& name, float val) {
	glUniform1f(getUniform(name), val); 
}

void Shader::set(const std::string& name, float val, float val2) {
	glUniform2f(getUniform(name), val, val2); 
}

void Shader::compile(const std::string& fileName, const GLenum& type, unsigned int& out) {
	char* shaderSource = static_cast<char*>(SDL_LoadFile(fileName.c_str(), nullptr));

	assert(shaderSource != nullptr && "Failed to read shader source");
	// SDL_Log("Source: %s\n", shaderSource);

	out = glCreateShader(type);
	glShaderSource(out, 1, &shaderSource, NULL);
	glCompileShader(out);
	SDL_free(shaderSource);

	// Error checking
	int success;
	glGetShaderiv(out, GL_COMPILE_STATUS, &success);
	if (!success) {
		int len = 0;
		glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);
		std::string log;
		log.resize(len);

		glGetShaderInfoLog(out, len, NULL, &log[0]);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile shader %s: %s\n",
						fileName.c_str(), log.c_str());
		ERROR_BOX("Failed to compile vertex shader");

		throw 1;
	}
}

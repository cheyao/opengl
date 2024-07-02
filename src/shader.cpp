#include "shader.hpp"

#include <SDL3/SDL.h>

#include "common.hpp"

Shader::Shader(const std::string& vertName, const std::string& fragName) {
	compile(vertName, GL_VERTEX_SHADER, mVertexShader);
	compile(fragName, GL_FRAGMENT_SHADER, mFragmentShader);

	mShaderProgram = glCreateProgram();
	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragmentShader);
	glLinkProgram(mShaderProgram);

	int success;
	glGetShaderiv(mShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];

		glGetShaderInfoLog(mShaderProgram, 512, NULL, infoLog);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile link shader: %s\n", infoLog);
		ERROR_BOX("Failed to link shader");

		throw 1;
	}
}

Shader::~Shader() {
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragmentShader);
	glDeleteProgram(mShaderProgram);
}

void Shader::activate() { glUseProgram(mShaderProgram); }

void Shader::compile(const std::string& fileName, GLenum type, unsigned int& out) {
	char* shaderSource = static_cast<char*>(SDL_LoadFile(fileName.c_str(), nullptr));
	out = glCreateShader(type);
	glShaderSource(out, 1, &shaderSource, NULL);
	glCompileShader(out);
	SDL_free(shaderSource);

	// Error checking
	int success;
	glGetShaderiv(out, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];

		glGetShaderInfoLog(out, 512, NULL, infoLog);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile shader %s: %s\n",
						fileName.c_str(), infoLog);
		ERROR_BOX("Failed to compile vertex shader");

		throw 1;
	}
}

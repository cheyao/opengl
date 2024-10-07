#include "opengl/shader.hpp"

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <version>

Shader::Shader(const std::string_view vertName, const std::string_view fragName, const std::string_view geomName)
	: mName(std::string(vertName) + ":" + std::string(fragName) + ":" + std::string(geomName)),
	  mShaderProgram(glCreateProgram()), mKeyHead(0) {
	const GLuint vertShader = compile(vertName, GL_VERTEX_SHADER);
	SDL_assert(glIsShader(vertShader) && "Shader.cpp: Vert shader not loaded correctly");

	const GLuint fragShader = compile(fragName, GL_FRAGMENT_SHADER);
	SDL_assert(glIsShader(fragShader) && "Shader.cpp: Frag shader not loaded correctly");

	GLuint geomShader = geomName.empty() ? 0 : compile(geomName, GL_GEOMETRY_SHADER);
	if (!geomName.empty()) {
		SDL_assert(geomShader != 0);
		SDL_assert(glIsShader(geomShader) && "Shader.cpp: Geom shader not loaded correctly");
	}

	glAttachShader(mShaderProgram, vertShader);
	glAttachShader(mShaderProgram, fragShader);
	if (!geomName.empty()) {
		glAttachShader(mShaderProgram, geomShader);
	}
	glLinkProgram(mShaderProgram);

	// First delete this to avoid mem leaks
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	if (!geomName.empty()) {
		glDeleteShader(geomShader);
	}

	GLint success = 0;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
	[[unlikely]] if (success == 0 || !glIsProgram(mShaderProgram)) {
		GLint len = 0;
		glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];

		glGetProgramInfoLog(mShaderProgram, 512, nullptr, &log[0]);

		log[len] = 0;

		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Shader.cpp: Failed to link shader: \n%s\n", log);
		ERROR_BOX("Failed to link shader");

		delete[] log;

		throw std::runtime_error("Shader.cpp: Failed to link shader");
	}

	SDL_assert(glIsProgram(mShaderProgram) &&
		   "Shader.cpp: Error compiling program, something wrong with code: should have been catched");

	bind("Matrices", 0);

#ifdef DEBUG
	GLint len = 0;
	glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);

	if (len == 0) {
		return;
	}

	GLchar* log = new GLchar[len + 1];

	log[len] = 0;

	glGetProgramInfoLog(mShaderProgram, 512, nullptr, &log[0]);
	SDL_Log("Log of shader compile: \n%s\n", log);
#endif
}

Shader::~Shader() { glDeleteProgram(mShaderProgram); }

void Shader::activate() const noexcept { glUseProgram(mShaderProgram); }

// Well the map with string view is complicated
// See discord discussion on C++ server:
// https://discord.com/channels/331718482485837825/331718580070645760/1291066117002891381
GLint Shader::getUniform(const std::string_view name) {
	// Bad for performance, but I need this
	SDL_assert([] {
		GLint prog = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		return static_cast<GLuint>(prog);
	}() == mShaderProgram &&
		   "The shader isn't activated!");

	// Safety checks
#ifdef __cpp_lib_string_contains
	SDL_assert(!name.contains(' '));
#endif
	SDL_assert(!(name[0] == 'g' && name[1] == 'l' && name[2] == '_'));
	SDL_assert(glIsProgram(mShaderProgram));

	if (!mPositions.contains(name)) {
		// Here we have to create a new element
		SDL_assert(mKeyHead < 16);
		mKeys[mKeyHead] = std::string(name);
		// Store the string
		mPositions[mKeys[mKeyHead]] = glGetUniformLocation(mShaderProgram, name.data());
		mKeyHead++;

		if (mPositions.at(name) == -1) {
			SDL_Log("\033[93mShader.cpp: Failed find uniform location \"%s\" for shader %s\033[0m",
				name.data(), mName.data());
		}
	}

	return mPositions.at(name);
}

void Shader::set(const std::string_view name, const GLboolean val) {
	glUniform1i(getUniform(name), static_cast<GLint>(val));
}
void Shader::set(const std::string_view name, const GLint val) { glUniform1i(getUniform(name), val); }
void Shader::set(const std::string_view name, const GLint val, const GLint val2) {
	glUniform2i(getUniform(name), val, val2);
}
void Shader::set(const std::string_view name, const GLuint val) { glUniform1ui(getUniform(name), val); }
void Shader::set(const std::string_view name, const GLfloat val) { glUniform1f(getUniform(name), val); }
void Shader::set(const std::string_view name, const GLdouble val) {
	glUniform1f(getUniform(name), static_cast<GLfloat>(val));
}
void Shader::set(const std::string_view name, const GLfloat val, const GLfloat val2) {
	glUniform2f(getUniform(name), val, val2);
}
void Shader::set(const std::string_view name, const GLfloat val, const GLfloat val2, const GLfloat val3) {
	glUniform3f(getUniform(name), val, val2, val3);
}
void Shader::set(const std::string_view name, const Eigen::Vector2f val) {
	glUniform2fv(getUniform(name), 1, val.data());
}
void Shader::set(const std::string_view name, const Eigen::Vector3f val) {
	glUniform3fv(getUniform(name), 1, val.data());
}
void Shader::set(const std::string_view name, const Eigen::Vector4f val) {
	glUniform4fv(getUniform(name), 1, val.data());
}
void Shader::set(const std::string_view name, const Eigen::Affine3f& mat, const GLboolean transpose) {
	glUniformMatrix4fv(getUniform(name), 1, transpose, mat.data());
}

void Shader::bind(const std::string_view name, const GLuint index) const {
	const GLuint blockIndex = glGetUniformBlockIndex(mShaderProgram, name.data());

	if (blockIndex == GL_INVALID_INDEX) {
		SDL_Log("\033[33mShader.cpp: Invalid block: %s\033[0m", name.data());

		return;
	}

	glUniformBlockBinding(mShaderProgram, blockIndex, index);
}

GLuint Shader::compile(const std::string_view fileName, const GLenum type) {
	char* shaderSource = static_cast<char*>(SDL_LoadFile(fileName.data(), nullptr));

	[[unlikely]] if (shaderSource == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Shader.cpp: Failed to read shader shource %s: %s\n",
				fileName.data(), SDL_GetError());

		ERROR_BOX("Failed to read assets");

		throw std::runtime_error("Shader.cpp: Failed to read shader source");
	}
	SDL_Log("Shader.cpp: Loading %s", fileName.data());

	SDL_assert(shaderSource[0] == '#' && "Shader.cpp: Checking file version");

#ifdef GLES
	// #version 410 core
	// #version 300 es
	shaderSource[9] = '3';
	shaderSource[10] = '0';
	shaderSource[11] = '0';
	shaderSource[12] = ' ';
	shaderSource[13] = 'e';
	shaderSource[14] = 's';
	shaderSource[15] = ' ';
	shaderSource[16] = ' ';
#endif

	GLuint out = glCreateShader(type);
	// Zero terminated, so no need for length
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

		log[len] = 0;

		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Shader.cpp: Failed to compile shader %s: \n%s\n",
				fileName.data(), log);

		ERROR_BOX("Failed to compile shader");

		delete[] log;

		throw std::runtime_error("Shader.cpp: Failed to compile shader");
	}

	SDL_assert(glIsShader(out) &&
		   "Shader.cpp: Error compiling shader, something wrong with code: should have been catched");

#ifdef DEBUG
	GLint len = 0;
	glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);

	if (len == 0) {
		return out;
	}

	GLchar* log = new GLchar[len + 1];

	log[len] = 0;

	glGetShaderInfoLog(out, 512, nullptr, &log[0]);
	SDL_Log("Log of shader compile: \n%s\n", log);
#endif

	return out;
}

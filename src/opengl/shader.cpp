#include "opengl/shader.hpp"
#include "utils.hpp"

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

Shader::Shader(const std::string_view& vertName, const std::string_view& fragName, const std::string_view& geomName)
	: mName(std::string(vertName) + ":" + std::string(fragName) + ":" + std::string(geomName)),
	  mShaderProgram(glCreateProgram()) {
	GLuint mVertexShader = compile(vertName, GL_VERTEX_SHADER);
	SDL_assert(glIsShader(mVertexShader) && "Shader.cpp: Vert shader not loaded correctly");

	GLuint mFragmentShader = compile(fragName, GL_FRAGMENT_SHADER);
	SDL_assert(glIsShader(mFragmentShader) && "Shader.cpp: Frag shader not loaded correctly");

	GLuint mGeometryShader = 0;
	if (!geomName.empty()) {
		mGeometryShader = compile(geomName, GL_GEOMETRY_SHADER);
		SDL_assert(glIsShader(mGeometryShader) && "Shader.cpp: Geom shader not loaded correctly");
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

void Shader::activate() const { glUseProgram(mShaderProgram); }

void Shader::setUniform(const std::string& name, std::function<void(GLint)> toCall) {
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
		mPositions[name] = glGetUniformLocation(mShaderProgram, name.data());

		if (mPositions[name] == -1) {
			SDL_Log("\033[93mShader.cpp: Failed find uniform location \"%s\" for shader %s\033[0m",
				name.data(), mName.data());

			return;
		}
	}

	if (mPositions[name] == -1) {
		return;
	}

	toCall(mPositions[name]);
}

void Shader::set(const std::string& name, const GLboolean val) {
	setUniform(name, std::bind(glUniform1i, std::placeholders::_1, static_cast<GLint>(val)));
}
void Shader::set(const std::string& name, const GLint val) {
	setUniform(name, std::bind(glUniform1i, std::placeholders::_1, val));
}
void Shader::set(const std::string& name, const GLuint val) {
	setUniform(name, std::bind(glUniform1ui, std::placeholders::_1, val));
}
void Shader::set(const std::string& name, const GLfloat val) {
	setUniform(name, std::bind(glUniform1f, std::placeholders::_1, val));
}
void Shader::set(const std::string& name, const GLdouble val) {
	setUniform(name, std::bind(glUniform1f, std::placeholders::_1, static_cast<GLfloat>(val)));
}
void Shader::set(const std::string& name, const GLfloat val, const GLfloat val2) {
	setUniform(name, std::bind(glUniform2f, std::placeholders::_1, val, val2));
}
void Shader::set(const std::string& name, const GLfloat val, const GLfloat val2, const GLfloat val3) {
	setUniform(name, std::bind(glUniform3f, std::placeholders::_1, val, val2, val3));
}
void Shader::set(const std::string& name, const Eigen::Vector2f val) {
	setUniform(name, std::bind(glUniform2f, std::placeholders::_1, val.x(), val.y()));
}
void Shader::set(const std::string& name, const Eigen::Vector3f val) {
	setUniform(name, std::bind(glUniform3f, std::placeholders::_1, val.x(), val.y(), val.z()));
}
void Shader::set(const std::string& name, const Eigen::Vector3f val, const GLfloat val2) {
	setUniform(name, std::bind(glUniform4f, std::placeholders::_1, val.x(), val.y(), val.z(), val2));
}
void Shader::set(const std::string& name, const Eigen::Vector4f val) {
	setUniform(name, std::bind(glUniform4f, std::placeholders::_1, val.x(), val.y(), val.z(), val.w()));
}
void Shader::set(const std::string& name, const Eigen::Affine3f& mat, const GLboolean transpose) {
	setUniform(name, std::bind(glUniformMatrix4fv, std::placeholders::_1, 1, transpose, mat.data()));
}

void Shader::bind(const std::string_view& name, const GLuint index) const {
	const GLuint blockIndex = glGetUniformBlockIndex(mShaderProgram, name.data());

	if (blockIndex == GL_INVALID_INDEX) {
		SDL_Log("Shader.cpp: Invalid block: %s", name.data());

		return;
	}

	glUniformBlockBinding(mShaderProgram, blockIndex, index);
}

GLuint Shader::compile(const std::string_view& fileName, const GLenum type) {
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

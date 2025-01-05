#include "opengl/shader.hpp"

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cinttypes>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

std::uint64_t crc32(const char* str, std::size_t len) {
	len = len - 1;

	std::uint64_t crc = 0xFFFFFFFF;

	for (std::size_t i = 0; i <= len; ++i) {
		crc = (crc >> 8) ^ crc_table[(crc ^ str[i]) & 0xFF];
	}

	return crc ^ 0xFFFFFFFF;
}

Shader::Shader() : mName(), mShaderProgram(glCreateProgram()) {}

bool Shader::load(std::string_view vertName, std::string_view fragName, std::string_view geomName) {
	mName = (std::string(vertName) + ":" + std::string(fragName) + ":" + std::string(geomName));

	const GLuint vertShader = compile(vertName, GL_VERTEX_SHADER);
	const GLuint fragShader = compile(fragName, GL_FRAGMENT_SHADER);
	[[unlikely]] if (vertShader == 0 || fragShader == 0) { return false; }

	GLuint geomShader = geomName.empty() ? 0 : compile(geomName, GL_GEOMETRY_SHADER);
	if (!geomName.empty()) {
		[[unlikely]] if (geomShader == 0) { return false; }
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
		std::unique_ptr<GLchar[]> log(new GLchar[len + 1]);

		glGetProgramInfoLog(mShaderProgram, 512, nullptr, &log[0]);

		log[len] = 0;

		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mShader.cpp: Failed to link shader: \n%s\n\033[0m",
				log.get());
		ERROR_BOX("Failed to link shader");

		return false;
	}

	bind("Matrices", 0);

	GLint maxLen, uniformCount;
	glGetProgramiv(mShaderProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(mShaderProgram, GL_ACTIVE_UNIFORMS, &uniformCount);

	std::vector<char> uniform(maxLen + 2);

	for (GLint i = 0; i < uniformCount; ++i) {
		GLsizei len;
		GLenum type;

		glGetActiveUniform(mShaderProgram, i, maxLen + 2, &len, nullptr, &type, uniform.data());
		len += 1;

		std::uint64_t hash = crc32(uniform.data(), len);

		mPositionCache[hash] = glGetUniformLocation(mShaderProgram, uniform.data());
	}

#ifdef DEBUG
	GLint len = 0;
	glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &len);

	if (len != 0) {
		std::unique_ptr<GLchar[]> log(new GLchar[len + 1]);

		log[len] = 0;

		glGetProgramInfoLog(mShaderProgram, 512, nullptr, log.get());
		SDL_Log("Log of shader compile: \n%s\n", log.get());
	}
#endif

	return true;
}

Shader::~Shader() { glDeleteProgram(mShaderProgram); }

void Shader::activate() const noexcept { glUseProgram(mShaderProgram); }

GLint Shader::getUniform(const std::uint64_t name) const {
	[[unlikely]] if (!mPositionCache.contains(name)) {
		SDL_Log("\033[93mShader.cpp: Failed find uniform location with hash \"%" PRIu64
			"\" for shader %s\033[0m",
			name, mName.data());

		return -1;
	}

	return mPositionCache[name];
}

void Shader::set(const std::uint64_t name, const GLboolean val) const {
	glUniform1i(getUniform(name), static_cast<GLint>(val));
}
void Shader::set(const std::uint64_t name, const GLint val) const { glUniform1i(getUniform(name), val); }
void Shader::set(const std::uint64_t name, const GLint val, const GLint val2) const {
	glUniform2i(getUniform(name), val, val2);
}
void Shader::set(const std::uint64_t name, const GLuint val) const { glUniform1ui(getUniform(name), val); }
void Shader::set(const std::uint64_t name, const GLfloat val) const { glUniform1f(getUniform(name), val); }
void Shader::set(const std::uint64_t name, const GLdouble val) const {
	glUniform1f(getUniform(name), static_cast<GLfloat>(val));
}
void Shader::set(const std::uint64_t name, const GLfloat val, const GLfloat val2) const {
	glUniform2f(getUniform(name), val, val2);
}
void Shader::set(const std::uint64_t name, const GLfloat val, const GLfloat val2, const GLfloat val3) const {
	glUniform3f(getUniform(name), val, val2, val3);
}
void Shader::set(const std::uint64_t name, const Eigen::Vector2f& val) const {
	glUniform2fv(getUniform(name), 1, val.data());
}
void Shader::set(const std::uint64_t name, const Eigen::Vector3f& val) const {
	glUniform3fv(getUniform(name), 1, val.data());
}
void Shader::set(const std::uint64_t name, const Eigen::Vector4f& val) const {
	glUniform4fv(getUniform(name), 1, val.data());
}
void Shader::set(const std::uint64_t name, const Eigen::Vector2i& val) const {
	glUniform2iv(getUniform(name), 1, val.data());
}
void Shader::set(const std::uint64_t name, const Eigen::Affine3f& mat, const GLboolean transpose) const {
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
	std::unique_ptr<char[], void (*)(void*)> shaderSource(static_cast<char*>(loadFile(fileName.data(), nullptr)),
							      SDL_free);

	[[unlikely]] if (shaderSource == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Shader.cpp: Failed to read shader shource %s: %s\n",
				fileName.data(), SDL_GetError());

		ERROR_BOX("Failed to read assets");

		return 0;
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
	const auto ptr = shaderSource.get();
	GLuint out = glCreateShader(type);
	// Zero terminated, so no need for length
	glShaderSource(out, 1, &ptr, nullptr);
	glCompileShader(out);

	// Error checking
	GLint success = 0;
	glGetShaderiv(out, GL_COMPILE_STATUS, &success);
	[[unlikely]] if (success == 0) {
		GLint len = 0;
		glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);
		std::unique_ptr<GLchar[]> log(new GLchar[len + 1]);

		glGetShaderInfoLog(out, len, nullptr, log.get());

		log[len] = 0;

		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Shader.cpp: Failed to compile shader %s: \n%s\n",
				fileName.data(), log.get());

		ERROR_BOX("Failed to compile shader");

		return 0;
	}

#ifdef DEBUG
	GLint len = 0;
	glGetShaderiv(out, GL_INFO_LOG_LENGTH, &len);

	if (len != 0) {
		std::unique_ptr<GLchar[]> log(new GLchar[len + 1]);

		log[len] = 0;

		glGetShaderInfoLog(out, len, nullptr, log.get());
		SDL_Log("Log of shader compile: \n%s\n", log.get());
	}
#endif

	return out;
}

#pragma once

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"

#include <string>
#include <string_view>
#include <unordered_map>

class Shader {
      public:
	explicit Shader(std::string_view vertName, std::string_view fragName, std::string_view geomName = "");
	Shader(Shader&&) = delete;
	Shader(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	~Shader();

	void activate() const noexcept;

	// set uniform
	void set(std::string_view name, GLboolean val) const;
	void set(std::string_view name, GLint val) const;
	void set(std::string_view name, GLint val, GLint val2) const;
	void set(std::string_view name, GLuint val) const;
	void set(std::string_view name, GLfloat val) const;
	void set(std::string_view name, GLdouble val) const;
	void set(std::string_view name, GLfloat val, GLfloat val2) const;
	void set(std::string_view name, GLfloat val, GLfloat val2, GLfloat val3) const;
	void set(std::string_view name, const Eigen::Vector2f& val) const;
	void set(std::string_view name, const Eigen::Vector3f& val) const;
	void set(std::string_view name, const Eigen::Vector4f& val) const;
	void set(std::string_view name, const Eigen::Vector2i& val) const;
	void set(std::string_view name, const Eigen::Affine3f& mat, GLboolean transpose = GL_FALSE) const;

	void bind(std::string_view name, GLuint index) const;

      private:
	[[nodiscard]] static GLuint compile(std::string_view fileName, GLenum type);

	GLint getUniform(std::string_view name) const;

	const std::string mName;
	const GLuint mShaderProgram;

	// Cache
	mutable std::unordered_map<std::string_view, GLint> mPositionCache;
	// Needed for the keys of the map
	// We can techincally go with just using string views atm since all out uniforms
	// are accessed using string litterals, but the code will break once we introduce ImGUI
	mutable std::array<std::string, 16> mKeys;
	mutable std::size_t mKeyHead;
};

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
	void set(std::string_view name, GLboolean val);
	void set(std::string_view name, GLint val);
	void set(std::string_view name, GLuint val);
	void set(std::string_view name, GLfloat val);
	void set(std::string_view name, GLdouble val);
	void set(std::string_view name, GLfloat val, GLfloat val2);
	void set(std::string_view name, GLfloat val, GLfloat val2, GLfloat val3);
	void set(std::string_view name, Eigen::Vector2f val);
	void set(std::string_view name, Eigen::Vector3f val);
	void set(std::string_view name, Eigen::Vector4f val);
	void set(std::string_view name, const Eigen::Affine3f& mat, GLboolean transpose = GL_FALSE);

	void bind(std::string_view name, GLuint index) const;

      private:
	[[nodiscard]] static GLuint compile(std::string_view fileName, GLenum type);

	GLint getUniform(std::string_view name);

	const std::string mName;
	const GLuint mShaderProgram;
	std::unordered_map<std::string_view, GLint> mPositions;
	// Needed for the keys of the map
	// We can techincally go with just using string views atm since all out uniforms
	// are accessed using string litterals, but the code will break once we introduce ImGUI
	std::array<std::string, 16> mKeys;
	std::size_t mKeyHead;
};

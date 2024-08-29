#pragma once

#include "third_party/Eigen/Dense"
#include "third_party/glad/glad.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

class Shader {
      public:
	explicit Shader(const std::string_view& vertName, const std::string_view& fragName,
			const std::string_view& geomName = "");
	Shader(Shader&&) = delete;
	Shader(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	~Shader();

	void activate() const;

	// set uniform
	void set(const std::string& name, const GLboolean val);
	void set(const std::string& name, const GLint val);
	void set(const std::string& name, const GLuint val);
	void set(const std::string& name, const GLfloat val);
	void set(const std::string& name, const GLdouble val);
	void set(const std::string& name, const GLfloat val, const GLfloat val2);
	void set(const std::string& name, const GLfloat val, const GLfloat val2, const GLfloat val3);
	void set(const std::string& name, const Eigen::Vector2f val);
	void set(const std::string& name, const Eigen::Vector3f val);
	void set(const std::string& name, const Eigen::Vector3f val, const GLfloat val2);
	void set(const std::string& name, const Eigen::Vector4f val);
	void set(const std::string& name, const Eigen::Affine3f& mat, const GLboolean transpose = GL_FALSE);

	void bind(const std::string_view& name, const GLuint index) const;

      private:
	[[nodiscard]] static GLuint compile(const std::string_view& fileName, const GLenum type);

	void setUniform(const std::string& name, std::function<void(GLint)> toCall);

	const std::string mName;
	const GLuint mShaderProgram;
	std::unordered_map<std::string, GLint> mPositions;
};

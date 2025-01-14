#pragma once

#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <string>

class Texture {
	friend class TextureManager;

      public:
	Texture(std::string_view path);
	explicit Texture(const Eigen::Vector2i& size, unsigned char* bitmap);
	explicit Texture(const Eigen::Vector2i& size);
	Texture(Texture&&) = delete;
	Texture(const Texture&) = delete;
	Texture& operator=(Texture&&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();

	virtual void activate(unsigned int num) const;

	int getWidth() const { return mWidth * 7; };
	int getHeight() const { return mHeight * 7; };
	Eigen::Vector2f getSize() const { return Eigen::Vector2f(mWidth, mHeight) * 7; }

      protected:
	[[nodiscard]] virtual bool load(bool srgb);

	GLuint mID;
	std::string name;
	int mWidth, mHeight;
};

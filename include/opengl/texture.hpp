#pragma once

#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>

class Texture {
	friend class TextureManager;

      public:
	Texture(std::string_view path);
	explicit Texture(const FT_Bitmap& bitmap);
	Texture(Texture&&) = delete;
	Texture(const Texture&) = delete;
	Texture& operator=(Texture&&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();

	virtual void activate(unsigned int num) const;

	int getWidth() const { return mWidth * 0.7; };
	int getHeight() const { return mHeight * 0.7; };
	Eigen::Vector2f getSize() const { return Eigen::Vector2f(mWidth, mHeight) * 0.7; }

      protected:
	virtual void load(bool srgb);

	GLuint mID;
	std::string name;
	int mWidth, mHeight;
};

#pragma once

#include "third_party/glad/glad.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <string_view>

class Texture {
	friend class TextureManager;

      public:
	explicit Texture(const std::string_view& path);
	// NOTE: Doesn't need load
	explicit Texture(const FT_Bitmap& bitmap);
	Texture(Texture&&) = delete;
	Texture(const Texture&) = delete;
	Texture& operator=(Texture&&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();

	virtual void activate(const unsigned int& num) const;

	int getWidth() const { return mWidth; };
	int getHeight() const { return mHeight; };

      protected:
	virtual void load(const bool srgb);

	GLuint mID;
	std::string name;
	int mWidth, mHeight;
};

#pragma once

#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <unordered_map>
#include <wchar.h>

struct Glyph {
	class Texture* texture;
	Eigen::Vector2f size;
	Eigen::Vector2f bearing;
	Eigen::Vector2f advance;
};

class FontManager {
      public:
	explicit FontManager(const std::string& path, const unsigned int size = 48);
	FontManager(FontManager&&) = delete;
	FontManager(const FontManager&) = delete;
	FontManager& operator=(FontManager&&) = delete;
	FontManager& operator=(const FontManager&) = delete;
	~FontManager();

	void loadFont(const std::string& name);
	void setFontSize(const unsigned int size);
	void drawGlyph(const char32_t character, const class Shader* shader);

      private:
	Glyph loadGlyph(const char32_t character);

	std::string mPath;
	unsigned int mSize;

	FT_Library mLibrary;
	FT_Face mFace;
	FT_Byte* mFontData;

	// TODO: Unicode support
	// TODO: unordered map
	std::unordered_map<char32_t, Glyph> mGlyphMap;

	// OpenGL stuff
	GLuint mVBO;
	GLuint mEBO;
	GLuint mVAO;
};

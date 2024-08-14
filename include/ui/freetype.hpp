#pragma once

#include "third_party/Eiegn/Core"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>

struct Gylph {
    unsigned int TextureID;
    Eiegn::Vector2f   Size;
    Eiegn::Vector2f   Bearing;
    unsigned int Advance;
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
	Gylph getGylph(const char character);

      private:
	std::string mPath;
	unsigned int mSize;

	FT_Library mLibrary;
	FT_Face mFace;

	// TODO: Unicode support
	// TODO: unordered map
	std::map<char, Gylph> mGylphMap;
};

#pragma once

#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H

class FontManager {
      public:
	explicit FontManager(const std::string& path);
	FontManager(FontManager&&) = delete;
	FontManager(const FontManager&) = delete;
	FontManager& operator=(FontManager&&) = delete;
	FontManager& operator=(const FontManager&) = delete;
	~FontManager();

	void loadFont(const std::string& name);

      private:
	std::string mPath;

	FT_Library mLibrary;
	FT_Face mFace;
};

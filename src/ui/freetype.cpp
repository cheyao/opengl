#include "ui/freetype.hpp"

#include "opengl/texture.hpp"
#include "third_party/Eigen/Core"
#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <stdexcept>
#include <wchar.h>

FontManager::FontManager(const std::string& path, const unsigned int size)
	: mPath(path + "assets" SEPARATOR "fonts" SEPARATOR), mSize(size), mLibrary(nullptr), mFace(nullptr) {
	if (FT_Init_FreeType(&mLibrary)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to init freetype");
		ERROR_BOX("Failed to init freetype, please reinstall your freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to init library");
	}
}

FontManager::~FontManager() {
	if (mFace != nullptr) {
		FT_Done_Face(mFace);
	}

	FT_Done_FreeType(mLibrary);
}

void FontManager::loadFont(const std::string& name) {
	assert(mLibrary != nullptr);

	if (mFace != nullptr) {
		FT_Done_Face(mFace);

		// Set to prevent double free if exception thrown
		mFace = nullptr;
	}

	size_t size = 0;
	const FT_Byte* data = static_cast<const FT_Byte*>(SDL_LoadFile((mPath + name).data(), &size));
	if (data == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: SDL failed to load file %s: %s", name.data(),
				SDL_GetError());
		ERROR_BOX("Failed load font, unknown file format, assets are probably corrupted");

		throw std::runtime_error("Freetype.cpp: Failed to load font unable to open file");
	}

	int status = FT_New_Memory_Face(mLibrary, data, size, 0, &mFace);
	if (status == FT_Err_Unknown_File_Format) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load font, unknown file format: %s",
				name.data());
		ERROR_BOX("Failed load font, unknown file format, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load font: unknown file format");
	} else if (status) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load font: %s", name.data());
		ERROR_BOX("Failed load font, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load font");
	}

	// Maybe `FT_Set_Char_Size`
	if (FT_Set_Pixel_Sizes(mFace, 0, mSize)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to set font size: %s", name.data());
		ERROR_BOX("Failed set font size, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to set font size");
	}

	mGylphMap.clear();
}

void FontManager::setFontSize(const unsigned int size) {
	mSize = size;

	if (mFace != nullptr) {
		FT_Set_Pixel_Sizes(mFace, 0, mSize);
	}

	mGylphMap.clear();
}

Glyph FontManager::getGylph(const char32_t character) {
	if (mGylphMap.contains(character)) {
		return mGylphMap.at(character);
	}

	FT_UInt index = FT_Get_Char_Index(mFace, character);
	if (index == 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
				"Freetype.cpp: Failed to load character: %x (undefined char code)", character);
		ERROR_BOX("Failed load character, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load character - undefine char code");
	}

	if (FT_Load_Glyph(mFace, character, FT_LOAD_RENDER)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load character: %x", character);
		ERROR_BOX("Failed load character, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load character");
	}

	if (mFace->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
		if (FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_NORMAL)) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load bitmap for character: %x",
					character);
			ERROR_BOX("Failed load character bitmap, please reinstall assets and the freetype library");

			throw std::runtime_error("Freetype.cpp: Failed to load character bitmap");
		}
	}

	Glyph glyph = {new Texture(mFace->glyph->bitmap),
		       Eigen::Vector2f(mFace->glyph->bitmap.width, mFace->glyph->bitmap.rows),
		       Eigen::Vector2f(mFace->glyph->bitmap_left, mFace->glyph->bitmap_top),
		       Eigen::Vector2f(mFace->glyph->advance.x, mFace->glyph->advance.y)};

	// assert(gylph.advance.y() == 0);

	mGlyphMap[character] = glyph;

	return glyph;
}

#include "ui/freetype.hpp"

#include "opengl/texture.hpp"
#include "third_party/Eigen/Core"
#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <stdexcept>
#include <wchar.h>

// FIXME: Verical text https://freetype.org/freetype2/docs/tutorial/step2.html
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
	FT_Byte* data = static_cast<FT_Byte*>(SDL_LoadFile((mPath + name).data(), &size));
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

		SDL_free(static_cast<void*>(data));

		throw std::runtime_error("Freetype.cpp: Failed to load font: unknown file format");
	} else if (status) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load font: %s", name.data());
		ERROR_BOX("Failed load font, please reinstall assets and the freetype library");

		SDL_free(static_cast<void*>(data));

		throw std::runtime_error("Freetype.cpp: Failed to load font");
	}
	SDL_free(static_cast<void*>(data));
	// TODO : https://freetype.org/freetype2/docs/tutorial/step1.html#section-1

	// Maybe `FT_Set_Char_Size`
	if (FT_Set_Pixel_Sizes(mFace, 0, mSize)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to set font size: %s", name.data());
		ERROR_BOX("Failed set font size, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to set font size");
	}

	mGlyphMap.clear();
}

void FontManager::setFontSize(const unsigned int size) {
	mSize = size;

	if (mFace != nullptr) {
		// TODO: fractions
		FT_Set_Pixel_Sizes(mFace, 0, mSize);
	}

	mGlyphMap.clear();
}

const Glyph& FontManager::getGlyph(const char32_t character) {
	if (mGlyphMap.contains(character)) {
		return mGlyphMap.at(character);
	}

	// FT_Get_Glyph & FT_Glyph_To_Bitmap?
	if (FT_Load_Char(mFace, character, FT_LOAD_RENDER)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load character: %x", character);
		ERROR_BOX("Failed load character, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load character");
	}

	/*
	if (mFace->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
		if (FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_NORMAL)) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load bitmap for character: %x",
					character);
			ERROR_BOX("Failed load character bitmap, please reinstall assets and the freetype library");

			throw std::runtime_error("Freetype.cpp: Failed to load character bitmap");
		}
	}
	*/

	Glyph glyph = {new Texture(mFace->glyph->bitmap),
		       Eigen::Vector2f(mFace->glyph->bitmap.width, mFace->glyph->bitmap.rows),
		       Eigen::Vector2f(mFace->glyph->bitmap_left, mFace->glyph->bitmap_top),
		       Eigen::Vector2f(mFace->glyph->advance.x, mFace->glyph->advance.y)};

	// assert(gylph.advance.y() == 0);

	mGlyphMap[character] = glyph;

	return mGlyphMap[character];
}

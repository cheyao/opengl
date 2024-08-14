#include "ui/freetype.hpp"

#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <stdexcept>

FontManager::FontManager(const std::string& path, const unsigned int size)
	: mPath(path + "assets" SEPARATOR "fonts" SEPARATOR), mSice(size), mLibrary(nullptr), mFace(nullptr) {
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

	FT_Set_Pixel_Sizes(mFace, 0, mSize);
}

void FontManager::setFontSize(const unsigned int size) {
	mSize = size;

	if (mFace != nullptr) {
		FT_Set_Pixel_Sizes(mFace, 0, mSize);
	}
}

Gylph FontManager::getGylph(const char character) {
    if (FT_Load_Char(mFace, character, FT_LOAD_RENDER)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load character: %c", character);
		ERROR_BOX("Failed load character, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load character");
    }

    Character character = {
        texture, 
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        face->glyph->advance.x
    };
    Characters.insert(std::pair<char, Character>(c, character));
}
}

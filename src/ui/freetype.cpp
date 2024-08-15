#include "ui/freetype.hpp"

#include "opengl/texture.hpp"
#include "third_party/Eigen/Core"
#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <stdexcept>
#include <wchar.h>

// PERF: Optimize with font atlas when using small fonts
// https://stackoverflow.com/questions/2071621/how-to-do-opengl-live-text-rendering-for-a-gui
// FIXME: Anti aliasing?
// FIXME: Lignatures
// FIXME: Maybe add support for emojis
// FIXME: Better unicode support
// FIXME: Some kind of fallback font
// FIXME: Verical text https://freetype.org/freetype2/docs/tutorial/step2.html
FontManager::FontManager(const std::string& path, const unsigned int size)
	: mPath(path + "assets" SEPARATOR "fonts" SEPARATOR), mSize(size), mLibrary(nullptr), mFace(nullptr),
	  mFontData(nullptr) {
	if (FT_Init_FreeType(&mLibrary)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to init freetype");
		ERROR_BOX("Failed to init freetype, please reinstall your freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to init library");
	}

	/*
	 * NOTE:
	 * This works by setting all the vectors to 1, and multiply the cords by the offset
	 * Not the best thing here.
	 */
	const float vertices[] = {
		0.0f, 0.0f, 0.0f, // TL
		0.0f, 1.0f, 0.0f, // BR
		1.0f, 0.0f, 0.0f, // TR
		1.0f, 1.0f, 0.0f  // BL
	};

	const float texturePos[] = {
		1.0f, 1.0f, // TR
		1.0f, 0.0f, // BR
		0.0f, 1.0f, // TL
		0.0f, 0.0f  // BL
	};

	const GLuint indices[] = {2, 1, 0,  // a
				  1, 2, 3}; // b
	
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() + normals.size() + texturePos.size()) * sizeof(float), nullptr,
		     GL_STATIC_DRAW);

	static_assert(std::is_same_v<decltype(positions[0]), decltype(normals[0])>);
	static_assert(std::is_same_v<decltype(texturePos[0]), decltype(normals[0])>);
	static_assert(std::is_same_v<decltype(texturePos[0]), const float&>);

	size_t offset = 0;

	// TODO: Non-hardcoded attrib pointer strides
	// TODO: Prettier
	glBindVertexArray(mVAO);
	assert(!positions.empty());
	{
		glBufferSubData(GL_ARRAY_BUFFER, offset, positions.size() * sizeof(float), positions.data());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(0);

		offset += positions.size() * sizeof(float);
	}

	if (!normals.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, normals.size() * sizeof(float), normals.data());
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(1);

		offset += normals.size() * sizeof(float);
	} else {
		SDL_Log("Mesh.cpp: Normals empty, ignored");
	}

	if (!texturePos.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, offset, texturePos.size() * sizeof(float), texturePos.data());
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(2);

		offset += texturePos.size() * sizeof(float);
	} else {
		SDL_Log("Mesh.cpp: Texture pos empty, ignored");
	}

	assert((positions.size() + normals.size() + texturePos.size()) * sizeof(float) == offset && "Missing data");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndicesCount * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

FontManager::~FontManager() {
	if (mFontData != nullptr) {
		SDL_free(static_cast<void*>(mFontData));
	}

	if (mFace != nullptr) {
		FT_Done_Face(mFace);
	}

	FT_Done_FreeType(mLibrary);

	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);
}

void FontManager::loadFont(const std::string& name) {
	assert(mLibrary != nullptr);

	size_t size = 0;
	FT_Byte* newFontData = static_cast<FT_Byte*>(SDL_LoadFile((mPath + name).data(), &size));
	if (newFontData == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: SDL failed to load file %s: %s", name.data(),
				SDL_GetError());
		ERROR_BOX("Failed load font, unknown file format, assets are probably corrupted");

		throw std::runtime_error("Freetype.cpp: Failed to load font unable to open file");
	}

	if (mFontData != nullptr) {
		SDL_free(static_cast<void*>(mFontData));
	}
	mFontData = newFontData;

	FT_Face newFace;
	int status = FT_New_Memory_Face(mLibrary, mFontData, size, 0, &newFace);
	if (status == FT_Err_Unknown_File_Format) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load font, unknown file format: %s",
				name.data());
		ERROR_BOX("Failed load font, unknown file format, please reinstall assets and the freetype library");

		SDL_free(static_cast<void*>(mFontData));

		throw std::runtime_error("Freetype.cpp: Failed to load font: unknown file format");
	} else if (status) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load font: %s", name.data());
		ERROR_BOX("Failed load font, please reinstall assets and the freetype library");

		SDL_free(static_cast<void*>(mFontData));

		throw std::runtime_error("Freetype.cpp: Failed to load font");
	}

	// TODO : https://freetype.org/freetype2/docs/tutorial/step1.html#section-1
	// TODO: Fractions with `FT_Set_Char_Size`
	if (FT_Set_Pixel_Sizes(newFace, 0, mSize)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to set font size: %s", name.data());
		ERROR_BOX("Failed set font size, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to set font size");
	}

	if (mFace != nullptr) {
		FT_Done_Face(mFace);
	}
	mFace = newFace;

	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}
	mGlyphMap.clear();
}

void FontManager::setFontSize(const unsigned int size) {
	mSize = size;

	if (mFace != nullptr) {
		FT_Set_Pixel_Sizes(mFace, 0, mSize);
	}

	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}
	mGlyphMap.clear();
}

const Glyph& FontManager::getGlyph(const char32_t character) {
	if (mGlyphMap.contains(character)) {
		return mGlyphMap.at(character);
	}

	assert(mFace != nullptr);

	// FT_Get_Glyph & FT_Glyph_To_Bitmap?
	if (FT_Load_Char(mFace, 'h', FT_LOAD_RENDER)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load character: 0x%x", character);
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

	glBindVertexArray(mVAO);
	mDrawFunc(GL_TRIANGLES, mIndicesCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	return mGlyphMap[character];
}

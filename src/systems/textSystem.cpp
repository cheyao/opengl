#include "systems/textSystem.hpp"

#include "game.hpp"
#include "opengl/shader.hpp"
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
// FIXME: Lignatures
// FIXME: Maybe add support for emojis
// FIXME: Better unicode support
// FIXME: Verical text https://freetype.org/freetype2/docs/tutorial/step2.html
TextSystem::TextSystem(Game* game, const unsigned int size, bool final)
	: mGame(game), mPath(game->fullPath("assets" SEPARATOR "fonts" SEPARATOR)), mSize(size), mLibrary(nullptr),
	  mFace(nullptr), mFontData(nullptr), mChild(nullptr) {
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
	constexpr const static float vertices[] = {
		0.0f, 0.0f, 0.0f, // TL
		0.0f, 1.0f, 0.0f, // BR
		1.0f, 0.0f, 0.0f, // TR
		1.0f, 1.0f, 0.0f  // BL
	};

	constexpr const static float texturePos[] = {
		0.0f, 1.0f, // TR
		0.0f, 0.0f, // BR
		1.0f, 1.0f, // TL
		1.0f, 0.0f  // BL
	};

	const static GLuint indices[] = {2, 1, 0,  // a
					 1, 2, 3}; // b
	static_assert(sizeof(indices) == 6 * sizeof(GLuint) && "Just a square, why not 6 indices?");

	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glGenVertexArrays(1, &mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texturePos), nullptr, GL_STATIC_DRAW);

	glBindVertexArray(mVAO);

	// Vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);

	// TexPos
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texturePos), texturePos);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<GLvoid*>(sizeof(vertices)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// TODO: Dyn load
	if (!final) {
		mChild = new TextSystem(path, game, size, true);
		mChild->loadFont("NotoSansCJK.ttc");
	}
}

TextSystem::~TextSystem() {
	if (mFontData != nullptr) {
		SDL_free(static_cast<void*>(mFontData));
	}

	if (mFace != nullptr) {
		FT_Done_Face(mFace);
	}

	FT_Done_FreeType(mLibrary);

	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}

	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);

	delete mChild;
}

void TextSystem::loadFont(const std::string& name) {
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

	// TODO: https://freetype.org/freetype2/docs/tutorial/step1.html#section-1
	// TODO: Fractions with `FT_Set_Char_Size`
	if (FT_Set_Pixel_Sizes(newFace, 0, 24)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to set font size: %s", name.data());
		ERROR_BOX("Failed set font size, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to set font size");
	}

	if (mFace != nullptr) {
		FT_Done_Face(mFace);
	}
	mFace = newFace;

	if (FT_Select_Charmap(mFace, FT_ENCODING_UNICODE)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to select unicode: %s", name.data());
		ERROR_BOX("Failed select unicode, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to select unicode");
	}

	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}
	mGlyphMap.clear();
}

void TextSystem::setFontSize(const unsigned int size) {
	mSize = size;

	if (mFace != nullptr) {
		FT_Set_Pixel_Sizes(mFace, 0, mSize);
	}

	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}
	mGlyphMap.clear();
}

void TextSystem::drawGlyph(const char32_t character, Shader* shader, const Eigen::Vector2f offset) {
	if (!mGlyphMap.contains(character)) {
		// TODO: try catch and sub char
		mGlyphMap[character] = loadGlyph(character);
	}

	const TextSystem::Glyph& glyph = mGlyphMap[character];

	if (glyph.size.x() <= 0 || glyph.size.y() <= 0) {
		return;
	}

	shader->activate();
	shader->set("letter", 0);
	shader->set("size", glyph.size);

	// TODO: Scale
	float x = offset.x() + glyph.bearing.x();
	float y = offset.y() - (glyph.size.y() - glyph.bearing.y());
	shader->set("offset", x, y);

	glyph.texture->activate(0);

	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

/*
Eigen::Vector2f TextSystem::getOffset(const char32_t character) {
	if (!mGlyphMap.contains(character)) {
		mGlyphMap[character] = loadGlyph(character);
	}

	return mGlyphMap[character].advance;
}

Eigen::Vector2f TextSystem::getSize(const char32_t character) {
	if (!mGlyphMap.contains(character)) {
		mGlyphMap[character] = loadGlyph(character);
	}

	return mGlyphMap[character].size;
}
*/

// FIXME: Loading two times
TextSystem::Glyph TextSystem::loadGlyph(const char32_t character) {
	assert(mFace != nullptr);

	const FT_UInt index = FT_Get_Char_Index(mFace, character);

	if (index == 0 && mChild != nullptr) {
		SDL_Log("Loading 0x%x fron fallback font", character);
		return mChild->loadGlyph(character);
	}

	if (FT_Load_Glyph(mFace, index, FT_LOAD_RENDER)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Freetype.cpp: Failed to load character: 0x%x", character);
		ERROR_BOX("Failed load character, please reinstall assets and the freetype library");

		throw std::runtime_error("Freetype.cpp: Failed to load character");
	}

	TextSystem::Glyph glyph = {(mFace->glyph->bitmap.rows > 0 && mFace->glyph->bitmap.width > 0)
			       ? new Texture(mFace->glyph->bitmap)
			       : nullptr,
		       Eigen::Vector2f(mFace->glyph->bitmap.width, mFace->glyph->bitmap.rows),
		       Eigen::Vector2f(mFace->glyph->bitmap_left, mFace->glyph->bitmap_top),
		       Eigen::Vector2f(mFace->glyph->advance.x >> 6, mFace->glyph->advance.y >> 6)};

	return glyph;
}

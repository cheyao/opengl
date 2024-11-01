#include "systems/textSystem.hpp"

#include "components.hpp"
#include "game.hpp"
#include "managers/localeManager.hpp"
#include "managers/systemManager.hpp"
#include "misc/sparse_set_view.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <wchar.h>

// PERF: Optimize with font atlas when using small fonts
// https://stackoverflow.com/questions/2071621/how-to-do-opengl-live-text-rendering-for-a-gui
// FIXME: Maybe add support for emojis
// FIXME: Better unicode support
TextSystem::TextSystem(Game* game, const unsigned int size, const bool final)
	: mGame(game), mPath(game->fullPath("fonts/")), mSize(size), mLibrary(nullptr), mFace(nullptr),
	  mFontData(nullptr), mChild(nullptr) {
	if (FT_Init_FreeType(&mLibrary)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "TextSystem.cpp: Failed to init freetype");
		ERROR_BOX("Failed to init freetype, please reinstall your freetype library");

		throw std::runtime_error("TextSystem.cpp: Failed to init library");
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

	mMesh = std::unique_ptr<Mesh>(new Mesh(vertices, {}, texturePos, indices, {}));

	// TODO: Dyn load CJK
	if (!final) {
		mChild = new TextSystem(game, size, true);
		mChild->loadFont("NotoSansCJK.ttc");
		loadFont("NotoSans.ttf");
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

	delete mChild;
}

void TextSystem::loadFont(const std::string& name) {
	SDL_assert(mLibrary != nullptr);

	size_t size = 0;
	FT_Byte* newFontData = static_cast<FT_Byte*>(SDL_LoadFile((mPath + name).data(), &size));
	if (newFontData == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mTextSystem.cpp: SDL failed to load file %s: %s",
				(mPath + name).data(), SDL_GetError());
		ERROR_BOX("Failed load font, unknown file format, assets are probably corrupted");

		throw std::runtime_error("TextSystem.cpp: Failed to load font unable to open file");
	}

	if (mFontData != nullptr) {
		SDL_free(static_cast<void*>(mFontData));
	}
	mFontData = newFontData;

	FT_Face newFace;
	const int status = FT_New_Memory_Face(mLibrary, mFontData, size, 0, &newFace);
	if (status == FT_Err_Unknown_File_Format) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
				"\x1B[31mTextSystem.cpp: Failed to load font, unknown file format: %s\033[0m",
				name.data());
		ERROR_BOX("Failed load font, unknown file format, please reinstall assets and the freetype library");

		SDL_free(static_cast<void*>(mFontData));

		throw std::runtime_error("TextSystem.cpp: Failed to load font: unknown file format");
	} else if (status) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "TextSystem.cpp: Failed to load font: %s", name.data());
		ERROR_BOX("Failed load font, please reinstall assets and the freetype library");

		SDL_free(static_cast<void*>(mFontData));

		throw std::runtime_error("TextSystem.cpp: Failed to load font");
	}

	// TODO: https://freetype.org/freetype2/docs/tutorial/step1.html#section-1
	// TODO: Fractions with `FT_Set_Char_Size`
	if (FT_Set_Pixel_Sizes(newFace, 0, 24)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mTextSystem.cpp: Failed to set font size: %s\033[0m",
				name.data());
		ERROR_BOX("Failed set font size, please reinstall assets and the freetype library");

		throw std::runtime_error("TextSystem.cpp: Failed to set font size");
	}

	if (mFace != nullptr) {
		FT_Done_Face(mFace);
	}
	mFace = newFace;

	if (FT_Select_Charmap(mFace, FT_ENCODING_UNICODE)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mTextSystem.cpp: Failed to select unicode: %s\033[0m",
				name.data());
		ERROR_BOX("Failed select unicode, please reinstall assets and the freetype library");

		throw std::runtime_error("TextSystem.cpp: Failed to select unicode");
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
	SDL_assert(mFace != nullptr);

	const TextSystem::Glyph& glyph = getGlyph(character);

	if (glyph.size.x() <= 0 || glyph.size.y() <= 0) {
		return;
	}

	// TODO: Scale
	Eigen::Affine3f model = Eigen::Affine3f::Identity();
	model.translate(Eigen::Vector3f(offset.x() + glyph.bearing.x(),
					offset.y() - (glyph.size.y() - glyph.bearing.y()), 0.0f));
	shader->set("model", model);
	shader->set("size", glyph.size);

	SDL_assert(glyph.texture != nullptr);
	glyph.texture->activate(0);

	mMesh->draw(shader);
}

TextSystem::Glyph& TextSystem::getGlyph(const char32_t character) {
	if (mGlyphMap.contains(character)) {
		return mGlyphMap[character];
	}

	SDL_assert(mFace != nullptr);

	// Maybe directly ask child?
	const FT_UInt index = FT_Get_Char_Index(mFace, character);

	if (index == 0) {
		if (mChild != nullptr) {
			// SDL_Log("Loading 0x%x from fallback font", character);
			return mChild->getGlyph(character);
		} else {
			SDL_Log("\x1B[31mTextSystem.cpp: Failed to load character: 0x%x (No matching glyph)\033[0m",
				character);
		}
	}

	if (FT_Load_Glyph(mFace, index, FT_LOAD_RENDER)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mTextSystem.cpp: Failed to load character: 0x%x\033[0m",
				character);
		ERROR_BOX("Failed load character, please reinstall assets and the freetype library");

		throw std::runtime_error("TextSystem.cpp: Failed to load character");
	}

	mGlyphMap[character] = {(mFace->glyph->bitmap.rows > 0 && mFace->glyph->bitmap.width > 0)
					? new Texture(mFace->glyph->bitmap)
					: nullptr,
				Eigen::Vector2f(mFace->glyph->bitmap.width, mFace->glyph->bitmap.rows),
				Eigen::Vector2f(mFace->glyph->bitmap_left, mFace->glyph->bitmap_top),
				Eigen::Vector2f(mFace->glyph->advance.x >> 6, mFace->glyph->advance.y >> 6)};

	return mGlyphMap[character];
}

void TextSystem::draw(Scene* scene) {
	glDisable(GL_DEPTH_TEST);

	Shader* shader = mGame->getSystemManager()->getShader("text.vert", "text.frag");
	shader->activate();
	shader->set("letter", 0);
	shader->set("textColor", 1.0f, 1.0f, 1.0f);

	for (const auto& [_, text, position] : scene->view<Components::text, Components::position>().each()) {
		auto offset = position.mPosition;

		for (const auto c : mGame->getLocaleManager()->get(text.mID)) {
			drawGlyph(c, shader, offset);

			offset += getGlyph(c).advance;
		}
	}
}

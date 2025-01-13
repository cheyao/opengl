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
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"
#include "third_party/stb_truetype.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <memory>
#include <span>
#include <utility>
#include <vector>
#include <wchar.h>

// PERF: Optimize with font atlas when using small fonts
// https://stackoverflow.com/questions/2071621/how-to-do-opengl-live-text-rendering-for-a-gui
// FIXME: Maybe add support for emojis
// FIXME: Better unicode support
TextSystem::TextSystem(const unsigned int size, const bool final) noexcept
	: mGame(Game::getInstance()), mPath(getBasePath() + "assets/fonts/"), mSize(size), mFont(),
	  mFontData(nullptr, SDL_free), mChild(nullptr) {
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
		loadFont("NotoSans.ttf");
	}
}

TextSystem::~TextSystem() {
	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}
}

void TextSystem::loadFont(const std::string& name) {
	std::size_t size;
	unsigned char* newFontData = static_cast<unsigned char*>(loadFile((mPath + name).data(), &size));
	if (newFontData == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mTextSystem.cpp: SDL failed to load file %s: %s",
				(mPath + name).data(), SDL_GetError());
		ERROR_BOX("Failed load font, unknown file format, assets are probably corrupted");

		return;
	}

	if (!stbtt_InitFont(&mFont, newFontData, 0)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "TextSystem.cpp: Failed to load font: %s", name.data());
		ERROR_BOX("Failed load font, please reinstall assets");

		return;
	}

	mFontData.reset(newFontData);

	for (const auto& [_, texture] : mGlyphMap) {
		delete texture.texture;
	}
	mGlyphMap.clear();
}

// Somehow this is taking a ton of time??
void TextSystem::drawGlyph(const char32_t character, Shader* shader, const Eigen::Vector2f& offset) {
	const TextSystem::Glyph& glyph = getGlyph(character);

	[[unlikely]] if (!glyph.texture) { return; }

	shader->set("offset"_u,
		    Eigen::Vector2f(offset.x() + glyph.bearing.x(), offset.y() - (glyph.size.y() - glyph.bearing.y())));

	glyph.texture->activate(0);

	mMesh->draw(shader);
}

TextSystem::Glyph& TextSystem::getGlyph(const char32_t character) {
	if (mGlyphMap.contains(character)) {
		return mGlyphMap[character];
	}

	int w, h, xo, yo, x0, y0, x1, y1;
	unsigned char* bitmap =
		stbtt_GetCodepointBitmap(&mFont, 0, stbtt_ScaleForPixelHeight(&mFont, 24), character, &w, &h, &xo, &yo);

	// get the bbox of the bitmap centered around the glyph origin; so the
	// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
	// the bitmap top left is (leftSideBearing*scale,iy0).
	stbtt_GetCodepointBitmapBox(&mFont, character, stbtt_ScaleForPixelHeight(&mFont, 24), stbtt_ScaleForPixelHeight(&mFont, 24), &x0, &y0, &x1, &y1);
	if (character == ' ') {
		x0 = 0;
		x1 = 7;
	}

	mGlyphMap[character] = {new Texture(Eigen::Vector2f(w, h), bitmap), Eigen::Vector2f(w, h),
				Eigen::Vector2f(xo, -yo), Eigen::Vector2f(x1-x0+xo, 0)};

	stbtt_FreeBitmap(bitmap, nullptr);

	return mGlyphMap[character];
}

void TextSystem::draw(Scene* scene) {
	Shader* shader = mGame->getSystemManager()->getShader("text.vert", "text.frag");
	const Eigen::Vector2f dimensions = mGame->getSystemManager()->getDemensions();
	shader->activate();
	shader->set("letter"_u, 0);
	shader->set("textColor"_u, COLOR);

	for (const auto& [_, text, position] : scene->view<Components::text, Components::position>().each()) {
		auto offset = position.mPosition;

		if (offset.x() == std::numeric_limits<float>::infinity()) {
			float len = 0;
			for (const auto c : mGame->getLocaleManager()->get(text.mID)) {
				len += getGlyph(c).advance.x();
			}

			offset.x() = dimensions.x() - len;
		}

		if (SDL_isnan(offset.x())) {
			float len = 0;
			for (const auto c : mGame->getLocaleManager()->get(text.mID)) {
				len += getGlyph(c).advance.x();
			}

			offset.x() = dimensions.x() / 2 - len / 2;
		}

		if (offset.y() == std::numeric_limits<float>::infinity()) {
			float height = 0;
			for (const auto c : mGame->getLocaleManager()->get(text.mID)) {
				height = SDL_max(height, getGlyph(c).size.y());
			}

			offset.y() = dimensions.y() - height;
		}

		if (SDL_isnan(offset.y())) {
			float height = 0;
			for (const auto c : mGame->getLocaleManager()->get(text.mID)) {
				height = SDL_max(height, getGlyph(c).size.y());
			}

			offset.y() = dimensions.y() / 2 - height / 2;
		}

		for (const auto c : mGame->getLocaleManager()->get(text.mID)) {
			drawGlyph(c, shader, offset);

			offset += getGlyph(c).advance;
		}
	}
}

void TextSystem::draw(const std::string_view str, const Eigen::Vector2f& o, const bool translate,
		      const Eigen::Vector3f& color) {
	Shader* shader = mGame->getSystemManager()->getShader("text.vert", "text.frag");
	shader->activate();
	shader->set("letter"_u, 0);
	shader->set("textColor"_u, color);

	Eigen::Vector2f offset = o;

	if (translate) {
		for (const auto c : mGame->getLocaleManager()->get(str)) {
			drawGlyph(c, shader, offset);

			offset += getGlyph(c).advance;
		}
	} else {
		for (const auto c : str) {
			drawGlyph(c, shader, offset);

			offset += getGlyph(c).advance;
		}
	}
}

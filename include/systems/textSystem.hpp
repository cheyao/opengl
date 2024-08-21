#pragma once

#include "systems/system.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <unordered_map>
#include <wchar.h>

class TextSystem : public System {
      public:
	explicit TextSystem(const std::string& path, class Game* game, const unsigned int size = 48,
			    bool final = false);
	TextSystem(TextSystem&&) = delete;
	TextSystem(const TextSystem&) = delete;
	TextSystem& operator=(TextSystem&&) = delete;
	TextSystem& operator=(const TextSystem&) = delete;
	~TextSystem();

	void loadFont(const std::string& name);
	// Size in 1/64 of a pixel
	void setFontSize(const unsigned int size);

	// [[nodiscard]] Eigen::Vector2f getOffset(const char32_t character);
	// [[nodiscard]] Eigen::Vector2f getSize(const char32_t character);

      private:
	struct Glyph {
		class Texture* texture;
		Eigen::Vector2f size;
		Eigen::Vector2f bearing;
		Eigen::Vector2f advance;
	};

	Glyph loadGlyph(const char32_t character);
	void drawGlyph(const char32_t character, class Shader* shader, const Eigen::Vector2f offset);

	class Game* mGame;

	const std::string mPath;
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

	TextSystem* mChild;
};

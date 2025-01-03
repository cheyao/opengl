#pragma once

#include "third_party/Eigen/Core"

#include <ft2build.h>
#include <string_view>
#include FT_FREETYPE_H
#include <memory>
#include <string>
#include <unordered_map>

class TextSystem {
      public:
	explicit TextSystem(const unsigned int size = 48, bool final = false) noexcept;
	TextSystem(TextSystem&&) = delete;
	TextSystem(const TextSystem&) = delete;
	TextSystem& operator=(TextSystem&&) = delete;
	TextSystem& operator=(const TextSystem&) = delete;
	~TextSystem();

	void loadFont(const std::string& name);
	// Size in 1/64 of a pixel
	void setFontSize(const unsigned int size);
	void draw(class Scene* scene);
	void draw(std::string_view str, const Eigen::Vector2f& offset, bool translate, const Eigen::Vector3f& color = COLOR);

      private:
	const static inline Eigen::Vector3f COLOR = Eigen::Vector3f(0.0f, 0.0f, 0.0f);

	struct Glyph {
		class Texture* texture;

		Eigen::Vector2f size;
		Eigen::Vector2f bearing;
		Eigen::Vector2f advance;
	};

	void drawGlyph(const char32_t character, class Shader* shader, const Eigen::Vector2f& offset);
	Glyph& getGlyph(const char32_t character);

	class Game* mGame;

	const std::string mPath;
	unsigned int mSize;

	FT_Library mLibrary;
	FT_Face mFace;
	FT_Byte* mFontData;

	std::unordered_map<char32_t, Glyph> mGlyphMap;

	std::unique_ptr<class Mesh> mMesh;

	TextSystem* mChild;
};

#include "ui/textComponent.hpp"

#include "game.hpp"
#include "opengl/texture.hpp"
#include "opengl/types.hpp"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include "ui/freetype.hpp"

// TODO: Size change
TextComponent::TextComponent(class UIScreen* owner, std::string_view text) : UIComponent(owner), mText(text) {
}

void TextComponent::draw([[maybe_unused]] const Shader* shader) {
	const Shader* textShader = mOwner->getGame()->getShader("text.vert", "text.frag");
	const Glyph& c = mOwner->getGame()->getGlyph(U'h');

	const std::vector<float> vertices = {
		0.0f,	    0.0f,
		0.0f, // Top Left
		0.0f,	    c.size.y(),
		0.0f, // Bottom right
		c.size.x(), 0.0f,
		0.0f, // Top Right
		c.size.x(), c.size.y(),
		0.0f // Bottom left
	};
	const std::vector<float> texturePos = {
		1.0f, 1.0f, // TR
		1.0f, 0.0f, // BR
		0.0f, 1.0f, // TL
		0.0f, 0.0f  // BL
	};
	const std::vector<GLuint> indices = {2, 1, 0,  // a
					     1, 2, 3}; // b
	const std::vector<std::pair<Texture* const, TextureType>> textures = {
		std::make_pair(c.texture, TextureType::DIFFUSE)};

	mMesh = new Mesh(vertices, {}, texturePos, indices, textures);

	(void)textShader;
}

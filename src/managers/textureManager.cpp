#include "managers/textureManager.hpp"

#include "components.hpp"
#include "game.hpp"
#include "items.hpp"
#include "managers/systemManager.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "third_party/glad/glad.h"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <unordered_map>
#include <version>

TextureManager::TextureManager()
	: mPath(getBasePath() + "assets/textures/"), mBlitzed(new bool[etoi(Components::Item::ITEM_COUNT)]) {
	// OpenGL wants this
	stbi_set_flip_vertically_on_load(false);
	get("missing-texture.png");

	mAtlas = new Texture(Eigen::Vector2i(1024, 1024));
	glGenFramebuffers(1, &mAtlasBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mAtlasBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mAtlas->mID, 0);
	constexpr const GLenum buffers = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to make texture atlas\033[0m");
		ERROR_BOX("Failed to make texture atlas! Check your opengl.");
	}
}

// TODO: Unloading when out of memory
TextureManager::~TextureManager() {
	Texture* const missing = get("missing-texture.png");

	for (const auto& [_, texture] : mTextures) {
		if (texture != missing) {
			delete texture;
		}
	}

	delete missing;
	delete mAtlas;
}

Texture* TextureManager::get(const std::string& name, const bool srgb) {
	if (mTextures.contains(name)) {
		return mTextures.at(name);
	}

	Texture* texture = new Texture(mPath + name);
	[[unlikely]] if (!texture->load(srgb)) {
		SDL_Log("Warining! Texture not found for %s", name.data());

		delete texture;
		texture = get("missing-texture.png");
	}

	mTextures[name] = texture;

	return texture;
}

class Texture* TextureManager::getAtlas() { return mAtlas; }

void TextureManager::blitzAtlas(Components::Item block) {
	// Atlas contains texture
	if (mBlitzed[etoi(block)]) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mAtlasBuffer);
	Shader* const shader = Game::getInstance()->getSystemManager()->getShader("blitz.vert", "block.frag");
	shader->activate();
	shader->set("texture_diffuse"_u, 0);
	shader->set("position"_u, 0);
}

void TextureManager::reload() {
	for (auto& [name, texture] : mTextures) {
		delete texture;
		auto* newTexture = new Texture(mPath + name);
		texture = newTexture;
	}
}

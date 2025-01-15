#include "managers/textureManager.hpp"

#include "components.hpp"
#include "game.hpp"
#include "items.hpp"
#include "managers/systemManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "registers.hpp"
#include "systems/UISystem.hpp"
#include "systems/renderSystem.hpp"
#include "third_party/glad/glad.h"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <cstdint>
#include <unordered_map>
#include <version>

TextureManager::TextureManager()
	: mPath(getBasePath() + "assets/textures/"), mBlitzed(new bool[etoi(Components::Item::ITEM_COUNT)]) {
	for (std::uint64_t i = 0; i < etoi(Components::Item::ITEM_COUNT); ++i) {
		mBlitzed[i] = 0;
	}

	// OpenGL wants this
	stbi_set_flip_vertically_on_load(false);
	get("missing-texture.png");

	mAtlas = new Texture(Eigen::Vector2i(1024, 1024));
	glGenFramebuffers(1, &mAtlasBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mAtlasBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mAtlas->mID, 0);

	constexpr const GLenum buffers = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to make texture atlas\033[0m");
		ERROR_BOX("Failed to make texture atlas! Check your opengl.");
	}

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
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
	// Alr activated in render system
	Shader* const shader = Game::getInstance()->getSystemManager()->getShader("blitz.vert", "block.frag");

	// Set this only once
	static bool s = false;
	if (!s) {
		shader->set("texture_diffuse"_u, 0);
		s = true;
	}
	shader->set("position"_u, static_cast<int>(etoi(block)));

	get(registers::TEXTURES.at(block))->activate(0);

	Game::getInstance()->getSystemManager()->getUISystem()->getMesh()->draw(shader);

	mBlitzed[etoi(block)] = true;
}

void TextureManager::reload() {
	for (auto& [name, texture] : mTextures) {
		delete texture;
		auto* newTexture = new Texture(mPath + name);
		texture = newTexture;
	}
}

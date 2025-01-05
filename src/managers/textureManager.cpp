#include "managers/textureManager.hpp"

#include "opengl/texture.hpp"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <unordered_map>
#include <version>

TextureManager::TextureManager() : mPath(getBasePath() + "assets/textures/") {
	// OpenGL wants this
	stbi_set_flip_vertically_on_load(false);
	get("missing-texture.png");
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

// TODO: Unloading when out of memory
TextureManager::~TextureManager() {
	Texture* const missing = get("missing-texture.png");

	for (const auto& [_, texture] : mTextures) {
		if (texture != missing) {
			delete texture;
		}
	}

	delete missing;
}

void TextureManager::reload() {
	for (auto& [name, texture] : mTextures) {
		delete texture;
		auto* newTexture = new Texture(mPath + name);
		texture = newTexture;
	}
}

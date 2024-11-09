#include "managers/textureManager.hpp"

#include "opengl/texture.hpp"
#include "third_party/stb_image.h"

#include <unordered_map>
#include <version>

TextureManager::TextureManager(const std::string& path) : mPath(path + "assets/textures/") {
	// OpenGL wants this
	stbi_set_flip_vertically_on_load(false);
}

Texture* TextureManager::get(const std::string& name, const bool srgb) {
	if (mTextures.contains(name)) {
		return mTextures.at(name);
	}

	Texture* texture = new Texture(mPath + name);
	texture->load(srgb);

	mTextures[name] = texture;

	return texture;
}

// TODO: Unloading when out of memory
TextureManager::~TextureManager() {
	for (const auto& [_, texture] : mTextures) {
		delete texture;
	}
}

void TextureManager::reload() {
	for (auto& [name, texture] : mTextures) {
		delete texture;
		auto* newTexture = new Texture(mPath + name);
		texture = newTexture;
	}
}

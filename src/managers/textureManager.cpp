#include "managers/textureManager.hpp"

#include "opengl/texture.hpp"

#include <unordered_map>

TextureManager::TextureManager(const std::string& path) : mPath(path) {}

Texture* TextureManager::get(const std::string& name) {
	if (mTextures.contains(name)) {
		return mTextures.at(name);
	}

	Texture* texture = new Texture(mPath + "assets/graphics/" + name);
	mTextures[name] = texture;
	return texture;
}

// TODO: Unloading when out of memory

TextureManager::~TextureManager() {
	for (auto& [_, texture] : mTextures) {
		delete texture;
	}
}

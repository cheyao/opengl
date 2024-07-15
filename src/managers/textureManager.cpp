#include "managers/textureManager.hpp"

#include "opengl/texture.hpp"

#include <algorithm>
#include <unordered_map>

#ifdef DEBUG
#include <filesystem>
#endif

TextureManager::TextureManager(const std::string& path) : mPath(path + "assets/graphics/") {}

Texture* TextureManager::get(const std::string& name) {
	if (mTextures.contains(name)) {
		return mTextures.at(name);
	}

	Texture* texture = new Texture(mPath + name);
	mTextures[name] = texture;

#ifdef DEBUG
	mLastEdit[texture] = std::filesystem::last_write_time(mPath + name);
#endif

	return texture;
}

// TODO: Unloading when out of memory

TextureManager::~TextureManager() {
	for (auto& [_, texture] : mTextures) {
		delete texture;
	}
}

void TextureManager::reload() {
	for (auto& [name, texture] : mTextures) {
#ifdef DEBUG
		if (std::filesystem::last_write_time(mPath + name) ==
			mLastEdit[texture]) {
			continue;
		}
#endif

		delete texture;
		Texture* newTexture = new Texture(mPath + name);
		texture = newTexture;

#ifdef DEBUG
		mLastEdit[texture] = std::filesystem::last_write_time(mPath + name);
#endif
	}
}

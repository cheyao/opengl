#include "managers/textureManager.hpp"

#include "opengl/cubemap.hpp"
#include "opengl/texture.hpp"
#include "utils.hpp"

#include <unordered_map>

#ifdef HOT
#include <filesystem>
#endif

TextureManager::TextureManager(const std::string& path) : mPath(path + "assets" + SEPARATOR + "textures" + SEPARATOR) {}

Texture* TextureManager::get(const std::string& name) {
	if (mTextures.contains(name)) {
		return mTextures.at(name);
	}

	Texture* texture;
#ifdef __cpp_lib_string_contains
	if (!name.contains('.')) {
#else
	if (name.find('.') == std::string::npos) {
#endif
		texture = new Cubemap(mPath + name + SEPARATOR);
	} else {
		texture = new Texture(mPath + name);
	}
	texture->load();

	mTextures[name] = texture;

#ifdef HOT
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

void TextureManager::reload(bool full) {
	for (auto& [name, texture] : mTextures) {
#ifdef HOT
		if (!full && std::filesystem::last_write_time(mPath + name) == mLastEdit[texture]) {
			continue;
		}
#endif

		delete texture;
		auto* newTexture = new Texture(mPath + name);
		texture = newTexture;

#ifdef HOT
		mLastEdit[texture] = std::filesystem::last_write_time(mPath + name);
#endif
		(void) full;
	}
}

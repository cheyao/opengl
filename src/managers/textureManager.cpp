#include "managers/textureManager.hpp"

#include "opengl/cubemap.hpp"
#include "opengl/texture.hpp"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <unordered_map>

TextureManager::TextureManager(const std::string& path) : mPath(path + "assets" SEPARATOR "textures" SEPARATOR) {
	// OpenGL wants this
	stbi_set_flip_vertically_on_load(true);
}

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
		delete texture;
		auto* newTexture = new Texture(mPath + name);
		texture = newTexture;
	}
}

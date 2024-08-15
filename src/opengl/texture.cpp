#include "opengl/texture.hpp"

#include "third_party/glad/glad.h"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <stdexcept>
#include <string_view>

Texture::Texture(const std::string_view& path) : name(path), mWidth(0), mHeight(0) {}

Texture::Texture(const FT_Bitmap& bitmap) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	SDL_Log("Loading texture %s", name.data());

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap.width, bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_Log("Loaded freetype bitmap: %dx%d", bitmap.width, bitmap.rows);
}

Texture::~Texture() {
	glDeleteTextures(1, &mID);

	SDL_Log("Unloading texture %s", name.data());
}

void Texture::activate(const unsigned int& num) const {
	/*
	[[unlikely]] if (!mLoaded) {
		this->load();
	}
	*/

	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, mID);
}

// NOTE: Maybe load on demand?
void Texture::load() {
	SDL_Log("Loading texture %s", name.data());

	size_t size = 0;
	unsigned char* source = static_cast<unsigned char*>(SDL_LoadFile(name.data(), &size));
	[[unlikely]] if (source == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to read texture shource: %s\n", name.data());

		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have enough memory");

		throw std::runtime_error("texture.cpp: Failed to read texture");
	}

	int channels = 0;
	unsigned char* data = stbi_load_from_memory(source, size, &mWidth, &mHeight, &channels, 0);
	SDL_free(source);

	[[unlikely]] if (data == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to decompress texture: %s\n", name.data());
		ERROR_BOX("Failed to load textures, the assets is corrupted or you don't have enough memory");

		throw std::runtime_error("texture.cpp: Failed to load texture");
	}

	GLenum format = GL_RGB;
	switch (channels) {
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		[[unlikely]] default:
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s:%d Unimplemented image format: %s: %d\n",
				     __FILE__, __LINE__, name.data(), channels);
			ERROR_BOX("Failed to recognise file color format, the assets is probably "
				  "corrupted");

			throw std::runtime_error("texture.cpp: Invalid enum");
	}

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);

	SDL_Log("Loaded texture %s: %d channels %dx%d", name.data(), channels, mWidth, mHeight);
}

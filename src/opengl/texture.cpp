#include "opengl/texture.hpp"

#include "third_party/glad/glad.h"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <stdexcept>
#include <string_view>

Texture::Texture(const std::string_view& path) : name(path), mWidth(0), mHeight(0) {
	stbi_set_flip_vertically_on_load(true);
}

Texture::~Texture() {
	glDeleteTextures(1, &mID);

	SDL_Log("Unloading texture %s", name.data());
}

void Texture::activate(const unsigned int& num) const {
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
	// unsigned char* data = stbi_load(name.data(), &width, &height, &channels, 0);

	[[unlikely]] if (data == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to decompress texture: %s\n", name.data());
		ERROR_BOX("Failed to load textures, the assets is corrupted or you don't have enough memory");

		throw std::runtime_error("texture.cpp: Failed to load texture");
	}

	GLenum format = GL_RGB;
	switch (channels) {
		// TODO: Gray scale
		case 1:
			format = GL_LUMINANCE;
			break;
		case 2:
			// FIXME: Souldn't be like this
			format = GL_RG;
			break;
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

	// SDL_free(source);
	stbi_image_free(data);

	SDL_Log("Loaded texture %s: %d channels %dx%d", name.data(), channels, mWidth, mHeight);
}

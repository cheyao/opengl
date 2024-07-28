#include "opengl/texture.hpp"

#include "utils.hpp"

#include <third_party/glad/glad.h>
#include <third_party/stb_image.h>

#include <SDL3/SDL.h>
#include <stdexcept>
#include <string_view>

Texture::Texture(const std::string_view& path) : name(path) {
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);

	[[unlikely]] if (data == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s\n", path.data());
		ERROR_BOX("Failed to load textures, the assets is corrupted or you don't "
				  "have enough memory");

		throw std::runtime_error("texture.cpp: Failed to load texture");
	}

	SDL_Log("Loaded texture %s: %d channels %dx%d", name.data(), channels, width, height);

	GLenum format = GL_RGB;
	switch (channels) {
		// TODO: Gray scale
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		[[unlikely]] default:
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s:%d Unimplemented image format: %s\n",
						 __FILE__, __LINE__, path.data());
			ERROR_BOX("Failed to recognise file color format, the assets is probably "
					  "corrupted");

			throw std::runtime_error("texture.cpp: Invalid enum");
	}

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// SDL_free(source);
	stbi_image_free(data);
}

Texture::~Texture() {
#ifndef ADDRESS
	glDeleteTextures(1, &mID);
#endif

	SDL_Log("Unloading texture %s", name.data());
}

void Texture::activate(const unsigned int& num) const {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, mID);
}

#include "texture.hpp"

#include "common.hpp"

#include <SDL3/SDL.h>

#include <stb_image.h>

#include <glad/glad.h>

Texture::Texture(const std::string& path, const bool& flip) {
	stbi_set_flip_vertically_on_load(flip);

	int width, height, channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

	if (data == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture!\n");
		ERROR_BOX("Failed to load textures, the assets is probably corrupted or you don't have "
				  "enough memory");

		throw 1;
	}

	SDL_Log("Loaded texture %s: %d channels %dx%d", path.c_str(), channels, width, height);

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum format;
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
						 __FILE__, __LINE__, path.c_str());
			ERROR_BOX("Failed to recognise file color format, the assets is probably corrupted");

			throw 1;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

Texture::~Texture() { glDeleteTextures(1, &mID); }

void Texture::activate(const unsigned int& num) {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, mID);
}

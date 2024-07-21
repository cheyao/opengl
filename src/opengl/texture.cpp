#include "opengl/texture.hpp"

#include "utils.hpp"

#include <third_party/glad/glad.h>
#include <third_party/stb_image.h>

#include <SDL3/SDL.h>
#include <stddef.h>
#include <string_view>

Texture::Texture(const std::string_view& path) : name(path) {
	/*
	size_t size;
	unsigned char* source = static_cast<unsigned char*>(SDL_LoadFile(path.data(), &size));
	[[unlikely]] if (source == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to read shader shource: %s\n",
						name.data());
		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have "
				  "enough memory");

		throw 1;
	}

	unsigned char* data = stbi_load_from_memory(source, size, &width, &height, &channels, 0);
	*/
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);

	[[unlikely]] if (data == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture!\n");
		ERROR_BOX("Failed to load textures, the assets is probably corrupted or you don't "
				  "have enough memory");

		throw 1;
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

			throw 1;
	}

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

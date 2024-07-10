#include "opengl/texture.hpp"

#include "utils.hpp"

#include <third_party/stb_image.h>
#include <third_party/glad/glad.h>

#include <stddef.h>
#include <SDL3/SDL.h>

Texture::Texture(const std::string& path, const bool& flip) {
	stbi_set_flip_vertically_on_load(flip);

	int width, height, channels;
	size_t size;
	unsigned char* source = static_cast<unsigned char*>(SDL_LoadFile(path.c_str(), &size));
	[[unlikely]] if (source == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to read shader shource: %s\n",
						path.c_str());
		ERROR_BOX("Failed to read assets");

		throw 1;
	}

	unsigned char* data = stbi_load_from_memory(source, size, &width, &height, &channels, 0);

	[[unlikely]] if (data == nullptr) {
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

	SDL_free(source);
	stbi_image_free(data);
}

Texture::~Texture() { glDeleteTextures(1, &mID); }

void Texture::activate(const unsigned int& num) const {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, mID);
}

#include "opengl/texture.hpp"

#include "third_party/glad/glad.h"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL3/SDL.h>
#include <cstddef>
#include <stdexcept>
#include <string_view>

Texture::Texture(const std::string_view path) : name(path), mWidth(0), mHeight(0) {}

Texture::Texture(const FT_Bitmap& bitmap)
	: name(string_format("from freetype bitmap %ux%u", bitmap.width, bitmap.rows)), mWidth(bitmap.width),
	  mHeight(bitmap.rows) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

#ifdef GLES
	// See https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/texImage2D
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, mWidth, mHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mWidth, mHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
#endif

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture() {
	glDeleteTextures(1, &mID);

	SDL_Log("Unloading texture %s", name.data());
}

void Texture::activate(const unsigned int num) const {
	SDL_assert(num < 80);

	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, mID);
}

// NOTE: Maybe load on demand?
void Texture::load(bool srgb) {
	SDL_Log("Loading texture %s", name.data());

	std::size_t size = 0;
	unsigned char* source = static_cast<unsigned char*>(SDL_LoadFile(name.data(), &size));
	[[unlikely]] if (source == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to read texture shource: %s\033[0m",
				name.data());

		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have enough memory");

		throw std::runtime_error("Texture.cpp: Failed to read texture");
	}

	int channels = 0;
	unsigned char* data = stbi_load_from_memory(source, size, &mWidth, &mHeight, &channels, 0);
	SDL_free(source);

	[[unlikely]] if (data == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to decompress texture: %s\033[0m", name.data());
		ERROR_BOX("Failed to load textures, the assets is corrupted or you don't have enough memory");

		throw std::runtime_error("Texture.cpp: Failed to load texture");
	}

	GLenum format = GL_RGB;
	GLenum intFormat = GL_SRGB;
	switch (channels) {
		case 3:
			format = GL_RGB;
#ifdef GLES
			intFormat = GL_RGB;
#else
			intFormat = GL_SRGB;
#endif

			break;
		case 4:
			format = GL_RGBA;
#ifdef GLES
			intFormat = GL_RGBA;
#else
			intFormat = GL_SRGB_ALPHA;
#endif

			break;
		[[unlikely]] default:
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31m%s:%d Unimplemented image format: %s: %d\033[0m",
				     __FILE__, __LINE__, name.data(), channels);
			ERROR_BOX("Failed to recognise file color format, the assets is probably "
				  "corrupted");

			throw std::runtime_error("Texture.cpp: Invalid enum");
	}

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	SDL_assert(mWidth > 0 && mHeight > 0);
	glTexImage2D(GL_TEXTURE_2D, 0, srgb ? intFormat : format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);

	SDL_Log("Loaded texture %s: %d channels %dx%d", name.data(), channels, mWidth, mHeight);
}

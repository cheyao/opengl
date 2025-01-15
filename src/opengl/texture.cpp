#include "opengl/texture.hpp"

#include "third_party/glad/glad.h"
#include "third_party/stb_image.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <memory>
#include <string_view>

// Load on demand
Texture::Texture(const std::string_view path) : name(path), mWidth(0), mHeight(0) {}

Texture::Texture(const Eigen::Vector2i& size, unsigned char* bitmap)
	: name(string_format("from freetype bitmap %ux%u", size.x(), size.y())), mWidth(size.x()), mHeight(size.y()) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	// See https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/texImage2D
	glTexImage2D(GL_TEXTURE_2D, 0,
#ifdef GLES
		     GL_R8,
#else
		     GL_RED,
#endif
		     mWidth, mHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture::Texture(const Eigen::Vector2i& size) {
	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x(), size.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture::~Texture() {
	glDeleteTextures(1, &mID);

	SDL_Log("Unloading texture %s", name.data());
}

void Texture::activate(const unsigned int num) const {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, mID);
}

// NOTE: Maybe load on demand?
bool Texture::load(bool srgb) {
	SDL_Log("Loading texture %s", name.data());

	std::size_t size = 0;
	const auto sfree = [](unsigned char* p) { SDL_free(p); };
	std::unique_ptr<unsigned char[], decltype(sfree)> source(
		static_cast<unsigned char*>(loadFile(name.data(), &size)), sfree);
	[[unlikely]] if (!source) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to read texture shource %s: %s\033[0m",
				name.data(), SDL_GetError());

		ERROR_BOX("Failed to read assets, your assets are corrupted or you dont't have enough memory");

		return false;
	}

	int channels = 0;
	const auto stbifree = [](unsigned char* p) { stbi_image_free(p); };
	std::unique_ptr<unsigned char[], decltype(stbifree)> data(
		stbi_load_from_memory(source.get(), size, &mWidth, &mHeight, &channels, 0), stbifree);

	[[unlikely]] if (!data) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "\x1B[31mFailed to decompress texture: %s\033[0m", name.data());
		ERROR_BOX("Failed to load textures, the assets is corrupted or you don't have enough memory");

		return false;
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

			break; // Recoverable error
	}

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	SDL_assert(mWidth > 0 && mHeight > 0);
	glTexImage2D(GL_TEXTURE_2D, 0, srgb ? intFormat : format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE,
		     data.get());
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SDL_Log("Loaded texture %s: %d channels %dx%d", name.data(), channels, mWidth, mHeight);

	return true;
}

#pragma once

#include "items.hpp"
#include "third_party/glad/glad.h"

#include <memory>
#include <string>
#include <unordered_map>

class TextureManager {
      public:
	explicit TextureManager();
	TextureManager(TextureManager&&) = delete;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	~TextureManager();

	class Texture* get(const std::string& name, const bool srgb = true);

	class Texture* getAtlas();
	void blitzAtlas(Components::Item block);

	void reload();

      private:
	const std::string mPath;

	std::unordered_map<std::string, class Texture*> mTextures;
	std::unique_ptr<bool[]> mBlitzed;
	class Texture* mAtlas;
	GLuint mAtlasBuffer;
};

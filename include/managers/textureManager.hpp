#pragma once

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

	void reload();

  private:
	const std::string mPath;

	std::unordered_map<std::string, class Texture*> mTextures;
};

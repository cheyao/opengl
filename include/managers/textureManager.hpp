#pragma once

#include <string>
#include <unordered_map>

class TextureManager {
  public:
	explicit TextureManager(const std::string& path);
	TextureManager(TextureManager&&) = delete;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	~TextureManager();

	class Texture* get(const std::string& name);

	void reload();

  private:
	std::unordered_map<std::string, class Texture*> mTextures;

	std::string mPath;
};

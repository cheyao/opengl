#pragma once

#include <unordered_map>
#include <string>

class TextureManager {
public:
	explicit TextureManager(const std::string& path);
	TextureManager(TextureManager &&) = delete;
	TextureManager(const TextureManager &) = delete;
	TextureManager &operator=(TextureManager &&) = delete;
	TextureManager &operator=(const TextureManager &) = delete;
	~TextureManager();

	class Texture* get(const std::string& texture);

private:
	std::unordered_map<std::string, class Texture*> mTextures;

	std::string mPath;
};


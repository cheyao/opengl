#pragma once

#include <string>
#include <unordered_map>

#ifdef DEBUG
#include <filesystem>
#endif

class TextureManager {
  public:
	explicit TextureManager(const std::string& path);
	TextureManager(TextureManager&&) = delete;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	~TextureManager();

	class Texture* get(const std::string& name);

	void reload(bool full = false);

  private:
	std::unordered_map<std::string, class Texture*> mTextures;

	std::string mPath;

#ifdef DEBUG
	std::unordered_map<class Texture*, std::filesystem::file_time_type> mLastEdit;
#endif
};

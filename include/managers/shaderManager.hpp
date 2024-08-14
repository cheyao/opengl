#pragma once

#include <string>
#include <unordered_map>

class ShaderManager {
  public:
	explicit ShaderManager(const std::string& path);
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
	~ShaderManager();

	class Shader* get(const std::string& vert, const std::string& frag, const std::string& geom = "");

	void reload();

  private:
	std::unordered_map<std::string, class Shader*> mTextures;

	std::string mPath;
};

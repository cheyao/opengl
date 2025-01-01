#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

class ShaderManager {
      public:
	explicit ShaderManager();
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
	~ShaderManager();

	class Shader* get(std::string_view vert, std::string_view frag, std::string_view geom = "");

	void reload();

#ifdef IMGUI
	void debugGui();
#endif

      private:
	const std::string mPath;

	std::unordered_map<std::string, class Shader*> mShaders;
};

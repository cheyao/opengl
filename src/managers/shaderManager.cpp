#include "managers/shaderManager.hpp"

#include "opengl/shader.hpp"
#include "utils.hpp"

#include <string>
#include <unordered_map>

ShaderManager::ShaderManager(const std::string& path)
	: mPath(path + "assets" + SEPARATOR + "shaders" + SEPARATOR) {}

Shader* ShaderManager::get(const std::string& vert, const std::string& frag) {
	assert(vert.find(':') == std::string::npos && frag.find(':') == std::string::npos);

	if (mTextures.contains(vert + ':' + frag)) {
		return mTextures.at(vert + ':' + frag);
	}

	Shader* shader = new Shader(mPath + vert, mPath + frag);
	mTextures[vert + ':' + frag] = shader;

#ifdef DEBUG
	mLastEdit[shader] = std::max(std::filesystem::last_write_time(mPath + vert),
								 std::filesystem::last_write_time(mPath + frag));
#endif

	return shader;
}

// TODO: Unloading when out of memory

ShaderManager::~ShaderManager() {
	for (auto& [_, shader] : mTextures) {
		delete shader;
	}
}

void ShaderManager::reload() {
	for (auto& [names, shader] : mTextures) {
		std::string frag = names;

		size_t pos = frag.find(':');
		std::string vert = frag.substr(0, pos);
		frag.erase(0, pos + 1);

#ifdef DEBUG
		if (std::max(std::filesystem::last_write_time(mPath + vert),
					 std::filesystem::last_write_time(mPath + frag)) == mLastEdit[shader]) {
			continue;
		}
		try {
			Shader* newTexture = new Shader(mPath + vert, mPath + frag);
			delete shader;
			shader = newTexture;
		} catch (...) {
		}

		mLastEdit[shader] = std::max(std::filesystem::last_write_time(mPath + vert),
									 std::filesystem::last_write_time(mPath + frag));
#else
		delete shader;
		Shader* newTexture = new Shader(mPath + vert, mPath + frag);
		shader = newTexture;
#endif
	}
}

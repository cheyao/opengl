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

	return shader;
}

// TODO: Unloading when out of memory

ShaderManager::~ShaderManager() {
	for (const auto& [_, shader] : mTextures) {
		delete shader;
	}
}

void ShaderManager::reload() {
	SDL_Log("Reloading shaders");

	for (auto& [names, shader] : mTextures) {
		std::string frag = names;

		const size_t pos = frag.find(':');
		std::string vert = mPath + frag.substr(0, pos);
		frag.erase(0, pos + 1);
		frag = mPath + frag;

#ifdef DEBUG
		try {
			Shader* newTexture = new Shader(vert, frag);

			delete shader;

			shader = newTexture;
		} catch (...) {
			SDL_Log("Error recompiling shaders");
		}

#else
		Shader* newTexture = new Shader(mPath + vert, mPath + frag);
		delete shader;
		shader = newTexture;
#endif
	}
}

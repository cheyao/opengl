#include "managers/shaderManager.hpp"

#include "opengl/shader.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stddef.h>
#include <stdexcept>
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
	for (const auto& [_, shader] : mTextures) {
		delete shader;
	}
}

void ShaderManager::reload(bool full) {
	SDL_Log("Reloading shaders");

	for (auto& [names, shader] : mTextures) {
		std::string frag = names;

		const size_t pos = frag.find(':');
		std::string vert = mPath + frag.substr(0, pos);
		frag.erase(0, pos + 1);
		frag = mPath + frag;

#ifdef DEBUG
		SDL_Log("Reloading %s:%s", vert.data(), frag.data());

		const auto lastEditTime = std::max(std::filesystem::last_write_time(vert),
										   std::filesystem::last_write_time(frag));
		if (!full && lastEditTime == mLastEdit[shader]) {
			continue;
		}

		try {
			Shader* newTexture = new Shader(vert, frag);

			delete shader;

			shader = newTexture;
		} catch (std::runtime_error error) {
			SDL_Log("Error recompiling shaders: %s", error.what());

			shader = this->get("default.vert", "default.frag");
		}

		mLastEdit[shader] = lastEditTime;
#else
		try {
			delete shader;
			Shader* newTexture = new Shader(vert, frag);
			shader = newTexture;
		} catch (std::runtime_error error) {
			shader = this->get("default.vert", "default.frag");
			throw error;
		}
#endif
	}
}

#include "managers/shaderManager.hpp"

#include "opengl/shader.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

#ifdef DEBUG
#include <filesystem>
#endif

ShaderManager::ShaderManager(const std::string& path)
	: mPath(path + "assets" + SEPARATOR + "shaders" + SEPARATOR) {}

Shader* ShaderManager::get(const std::string& vert, const std::string& frag, const std::string& geom) {
	assert(!vert.contains(':') && !frag.contains(':') && !geom.contains(':'));

	std::string concated = (vert + ':').append(frag).append(":").append(geom);
	// Using append avoids copies, this function will be called a couple times per loop
	if (mTextures.contains(concated)) {
		return mTextures.at(concated);
	}

	Shader* shader = new Shader(mPath + vert, mPath + frag, geom.empty() ? geom : mPath + geom);

	mTextures[concated] = shader;

#ifdef DEBUG
	// FIXME: This somehow doesn't work (Incompatabilities with SDL?)
	mLastEdit[shader] = std::max(std::filesystem::last_write_time(mPath + vert),
								 std::filesystem::last_write_time(mPath + frag));
#endif

	return shader;
}

// TODO: Unloading when out of memory

ShaderManager::~ShaderManager() {
	// Default shader might get used multiple times
	Shader* def = this->get("default.vert", "default.frag");

	for (const auto& [_, shader] : mTextures) {
		if (shader == def) {
			continue;
		}

		delete shader;
	}

	delete def;
}

void ShaderManager::reload(bool full) {
	SDL_Log("Reloading shaders");

	for (auto& [names, shader] : mTextures) {
		const size_t pos = names.find(':');
		const size_t pos2 = names.find(':', pos + 1);
		std::string vert = mPath + names.substr(0, pos);
		std::string frag = mPath + names.substr(pos + 1, pos2 - pos - 1);
		std::string geom = names.substr(pos2 + 1, names.size() - pos);
		geom = geom.empty() ? "" : mPath + geom;

		SDL_Log("%s:%s:%s", vert.data(), frag.data(), geom.data());

#ifdef DEBUG
		SDL_Log("Reloading %s:%s", vert.data(), frag.data());

		// TODO: Some wierd vim write stuff
		// FIXME: I don't fucking know why this doesn't work
		std::filesystem::file_time_type lastEditTime;
		lastEditTime = std::max(std::filesystem::last_write_time(vert),
									   std::filesystem::last_write_time(frag));

		if (!full && lastEditTime == mLastEdit[shader]) {
			continue;
		}

		try {
			Shader* newTexture = new Shader(vert, frag, geom);

			delete shader;

			shader = newTexture;
		} catch (std::runtime_error error) {
			SDL_Log("Error recompiling shaders: %s", error.what());

			shader = this->get("default.vert", "default.frag");
		}

		/*
		mLastEdit[shader] = lastEditTime;
		*/
		(void) full;
#else
		try {
			delete shader;
			Shader* newTexture = new Shader(vert, frag, geom);
			shader = newTexture;
		} catch (std::runtime_error error) {
			shader = this->get("default.vert", "default.frag");
			throw error;
		}
#endif
	}
}

#include "managers/shaderManager.hpp"

#include "opengl/shader.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

ShaderManager::ShaderManager(const std::string& path) : mPath(path + "assets" SEPARATOR "shaders" SEPARATOR) {}

Shader* ShaderManager::get(const std::string& vert, const std::string& frag, const std::string& geom) {
#ifdef __cpp_lib_string_contains
	// Ugh don't mind assering when the libc++ isn't up to date
	assert(!vert.contains(':') && !frag.contains(':') && !geom.contains(':'));
#endif

	std::string concated = (vert + ':').append(frag).append(":").append(geom);
	// Using append avoids copies, this function will be called a couple times per loop
	if (mTextures.contains(concated)) {
		return mTextures.at(concated);
	}

	Shader* shader = new Shader(mPath + vert, mPath + frag, geom.empty() ? geom : mPath + geom);

	mTextures[concated] = shader;

	return shader;
}

// TODO: Unloading when out of memory
ShaderManager::~ShaderManager() {
	// Default shader might get used multiple times
	Shader* def = this->get("default.vert", "default.frag");

	for (const auto& [_, shader] : mTextures) {
		if (shader != def) {
			delete shader;
		}
	}

	delete def;
}

void ShaderManager::reload() {
	SDL_Log("Reloading shaders");

	for (auto& [names, shader] : mTextures) {
		const size_t pos = names.find(':');
		const size_t pos2 = names.find(':', pos + 1);
		const std::string vert = mPath + names.substr(0, pos);
		const std::string frag = mPath + names.substr(pos + 1, pos2 - pos - 1);
		std::string geom = names.substr(pos2 + 1, names.size() - pos);
		geom = geom.empty() ? "" : mPath + geom;

#ifdef DEBUG
		SDL_Log("Reloading %s:%s", vert.data(), frag.data());

		try {
			Shader* newTexture = new Shader(vert, frag, geom);

			delete shader;

			shader = newTexture;
		} catch (const std::runtime_error& error) {
			SDL_Log("Error recompiling shaders: %s", error.what());

			shader = this->get("default.vert", "default.frag");
		}
#else
		try {
			delete shader;

			Shader* newTexture = new Shader(vert, frag, geom);

			shader = newTexture;
		} catch (const std::runtime_error& error) {
			shader = this->get("default.vert", "default.frag");

			throw error;
		}
#endif
	}
}

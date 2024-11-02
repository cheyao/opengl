#include "managers/shaderManager.hpp"

#include "imgui.h"
#include "opengl/shader.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <version>

ShaderManager::ShaderManager(const std::string_view path) : mPath(std::string(path) + "assets/shaders/") {
	mShaders["default.vert:default.frag"] = nullptr;
}

Shader* ShaderManager::get(std::string_view vert, std::string_view frag, std::string_view geom) {
	SDL_assert(!(vert == "default.vert" && frag == "default.frag"));
#ifdef __cpp_lib_string_contains
	// Ugh don't mind assering when the libc++ isn't up to date
	SDL_assert(!vert.contains(':') && !frag.contains(':') && !geom.contains(':'));
#endif

	std::string concated = std::string(vert) + ':';
	concated += frag;
	concated += ':';
	concated += geom;

	// Using append avoids copies, this function will be called a couple times per loop
	if (mShaders.contains(concated)) {
		return mShaders.at(concated);
	}

	Shader* shader = nullptr;
	try {
		shader = new Shader(mPath + std::string(vert), mPath + std::string(frag),
				    geom.empty() ? geom : mPath + std::string(geom));
	} catch (const std::runtime_error& error) {
		SDL_LogError(
			SDL_LOG_CATEGORY_RENDER,
			"\x1B[31mShaderManager.cpp: Error compiling shader %s, falling back to default shader\033[0m",
			concated.data());

		if (mShaders["default.vert:default.frag"] == nullptr) {
			mShaders["default.vert:default.frag"] =
				new Shader(mPath + "default.vert", mPath + "default.frag");
		}
		shader = mShaders["default.vert:default.frag"];
	}

	mShaders[concated] = shader;

	return shader;
}

// TODO: Unloading when out of memory
ShaderManager::~ShaderManager() {
	// Default shader might get used multiple times
	Shader* def = mShaders["default.vert:default.frag"];

	for (const auto& [_, shader] : mShaders) {
		if (shader != def) {
			delete shader;
		}
	}

	delete def;
}

void ShaderManager::reload() {
	SDL_Log("Reloading shaders");

	for (auto& [names, shader] : mShaders) {
		const std::size_t pos = names.find(':');
		const std::size_t pos2 = names.find(':', pos + 1);
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

#ifdef IMGUI
void ShaderManager::debugGui() {
	// TODO: Live edit GUI
	static bool liveEdit = false;
	ImGui::Begin("Main menu");
	ImGui::Checkbox("Shader Editor", &liveEdit);
	ImGui::End();

	if (!liveEdit) {
		return;
	}

	static std::size_t selection = 0;

	std::vector<std::string> shaders;

	for (const auto& [names, _] : mShaders) {
		const std::size_t pos = names.find(':');
		const std::size_t pos2 = names.find(':', pos + 1);
		const std::string vert = names.substr(0, pos);
		const std::string frag = names.substr(pos + 1, pos2 - pos - 1);
		std::string geom = names.substr(pos2 + 1, names.size() - pos);

		shaders.emplace_back(vert);
		shaders.emplace_back(frag);

		if (!geom.empty()) {
			shaders.emplace_back(geom);
		}
	}

	ImGui::Begin("Shader Editor");
	if (ImGui::BeginCombo("file", shaders[selection].data())) {
		for (std::size_t i = 0; i < shaders.size(); i++) {
			const bool is_selected = (selection == i);
			if (ImGui::Selectable(shaders[i].data(), is_selected)) {
				selection = i;
			}

			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::End();
}
#endif

#include "managers/shaderManager.hpp"

#include "imgui.h"
#include "opengl/shader.hpp"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <version>

ShaderManager::ShaderManager() : mPath(getBasePath() + "assets/shaders/") {
	mShaders["default.vert:default.frag:"] = nullptr;
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

		if (mShaders["default.vert:default.frag:"] == nullptr) {
			mShaders["default.vert:default.frag:"] =
				new Shader(mPath + "default.vert", mPath + "default.frag");
		}

		shader = mShaders["default.vert:default.frag:"];
	}

	mShaders[concated] = shader;

	return shader;
}

/*
Thanks @dragonslayer0531 for typing all this out :D

const auto keys = _mm_set_epi32(value); // broadcast the value to search for across a vector register

// Assumes the size of the vector to search against is a multiple of 8 and is properly aligned
for (auto i = 0; i < vec.size(); i += 8) {
  // Load 8 elements into registers
  const auto v1 = _mm_loadu_si128(reinterpret_cast<const _m128i*>(&vec[i + 0])); // Load 4 elements into a vector
register const auto v2 = _mm_loadu_si128(reinterpret_cast<const _m128i*>(&vec[i + 4])); // Load the next 4 elements into
another vector register

  // Compare the elements
  const auto c1 = _mm_cmpeq_epi32(v1, keys);
  const auto c2 = _mm_cmpeq_epi32(v2, keys);

  // Get a mask representing the result of the comparison
  const auto mask = _mm_movemask_epi8(_mm_packs_epi32(c1, c2));

  // If the mask is not 0, then we found what we're looking for
  if (mask != 0) {
    return i + std::countr_zero(mask);
  }
}

// Handle the case where it's not a multiple of 8 with "regular" code
// */

// TODO: Unloading when out of memory
ShaderManager::~ShaderManager() {
	// Default shader might get used multiple times
	Shader* def = mShaders["default.vert:default.frag:"];

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
			shader = nullptr;

			shader = newTexture;
		} catch (const std::runtime_error& error) {
			SDL_Log("Error recompiling shaders: %s", error.what());

			shader = this->get("default.vert", "default.frag");
		}
#else
		try {
			delete shader;
			shader = nullptr;

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
	static bool liveEdit = false;
	ImGui::Begin("Developer menu");
	ImGui::Checkbox("Shader Editor", &liveEdit);
	ImGui::End();

	if (!liveEdit) {
		return;
	}

	std::vector<std::string> shaders;

	for (const auto& [names, _] : mShaders) {
		const std::size_t pos = names.find(':');
		const std::size_t pos2 = names.find(':', pos + 1);
		const std::string vert = names.substr(0, pos);
		const std::string frag = names.substr(pos + 1, pos2 - pos - 1);
		const std::string geom = names.substr(pos2 + 1, names.size() - pos);

		shaders.emplace_back(vert);
		shaders.emplace_back(frag);

		if (!geom.empty()) {
			shaders.emplace_back(geom);
		}
	}

	static std::size_t selection = 0;
	bool update = false;

	// Make the vector unique
	ImGui::Begin("Shader Editor");
	if (ImGui::BeginCombo("file", shaders[selection].data())) {
		for (std::size_t i = 0; i < shaders.size(); i++) {
			const bool is_selected = (selection == i);
			if (ImGui::Selectable(shaders[i].data(), is_selected)) {
				selection = i;
				update = true;
			}

			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (!update) {
		ImGui::End();
		return;
	}

	// TODO: Live edit GUI

	ImGui::End();
}
#endif

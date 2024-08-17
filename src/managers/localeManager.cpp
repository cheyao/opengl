#include "managers/localeManager.hpp"

#include "third_party/json.hpp"
#include "utils.hpp"

#include <string>

LocaleManager::LocaleManager(const std::string& path)
	: mLocaleDir(path + "assets" SEPARATOR "strings" SEPARATOR), mLocale("en-US") {
	loadLocale();
}

std::u32string LocaleManager::get(std::string_view id) {
	/*
	if (!mLocaleData.contains(id)) {
		SDL_Log("\x1B[31mLocaleManager.cpp: Error! Unknown id %s\033[0m", id.data());

#ifdef DEBUG
		throw std::runtime_error("LocaleManager.cpp: Unknown id!");
#endif

			return U"";
	}
	*/

	return mLocaleData[id.data()].get<std::u32string>();
}

void LocaleManager::loadLocale() {
	SDL_Log("LocalManager.cpp: Loading %s", mLocale.data());

	std::string main;

#ifdef __cpp_lib_string_contains
	if (mLocale.contains('-')) {
#else
	if (mLocale.find('-') != std::string::npos) {
#endif
		const size_t pos = mLocale.find('-');
		main = mLocale.substr(0, pos);
	} else {
		main = mLocale;
	}

	char* localeData = static_cast<char*>(SDL_LoadFile((mLocaleDir + main + ".json").data(), nullptr));
	if (localeData == nullptr) {
		localeData = static_cast<char*>(SDL_LoadFile((mLocaleDir + mLocale + ".json").data(), nullptr));

		if (localeData == nullptr) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
					"LocaleManager.cpp: Failed to find/read locale shource %s: %s\n",
					(mLocaleDir + main + ".json").data(), SDL_GetError());

			ERROR_BOX("Failed to find/read locale");

			throw std::runtime_error("LocaleManager.cpp: Failed to find/read locale");
		}
	}

	mLocaleData = nlohmann::json::parse(localeData);

	SDL_Log("LocalManager.cpp: Locale %s version %d", mLocale.data(), mLocaleData["version"].get<int>());

	SDL_Log("LocalManager.cpp: Successfully loaded locale %s", mLocale.data());
}

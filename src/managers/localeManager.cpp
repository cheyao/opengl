#include "managers/localeManager.hpp"

#include "third_party/json.hpp"
#include "utils.hpp"

#include <codecvt>
#include <stdexcept>
#include <string>

// TODO: SDL_EVENT_LOCALE_CHANGED

LocaleManager::LocaleManager(const std::string& path) : mLocaleDir(path + "assets" SEPARATOR "strings" SEPARATOR) {
	int c = 0;
	SDL_Locale** loc = SDL_GetPreferredLocales(&c);
	std::string locList = "";
	for (int i = 0; i < c; ++i) {
		locList += loc[i]->language;
		if (loc[i]->language != 0) {
			locList += '-';
			locList += loc[i]->country;
		}
		locList += ' ';
	}

	SDL_Log("LocaleManager.cpp: Found locales %s", locList.data());

	for (int i = 0; i < c; ++i) {
		std::string l;
		l += loc[i]->language;
		if (loc[i]->language != 0) {
			l += '-';
			l += loc[i]->country;
		}

		SDL_Log("LocaleManager.cpp: Tring to load locale %s", l.data());

		mLocale = l;

		try {
			loadLocale();
		} catch (const std::runtime_error& error) {
			SDL_Log("LocaleManager.cpp: Failed to load locale %s: %s", l.data(), error.what());
			continue;
		}

		SDL_Log("LocaleManager.cpp: Successfully loaded locale %s", l.data());

		return;
	}

	mLocale = "en-US";
	SDL_Log("\x1B[31mLocaleManager.cpp: Failed to find valid locale! Falling back to english.\033[0m");
	loadLocale();
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

std::u32string LocaleManager::get(std::string_view id) {
	if (!mLocaleData.contains(id)) {
		SDL_Log("\x1B[31mLocaleManager.cpp: Error! Unknown id %s\033[0m", id.data());

#ifdef DEBUG
		throw std::runtime_error("LocaleManager.cpp: Unknown id!");
#endif

		return U"";
	}

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
	try {
		return conv.from_bytes(mLocaleData[id].get<std::string>());
	} catch (const std::range_error& e) {
		SDL_Log("\x1B[31mLocaleManager.cpp: Error! Unknown id %s\033[0m", id.data());

#ifdef DEBUG
		throw std::runtime_error("LocaleManager.cpp: Unable to convert to UTF32!");
#endif

		return conv.from_bytes(mLocaleData[id].get<std::string>().substr(0, conv.converted()));
	}
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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

	char* localeData = static_cast<char*>(SDL_LoadFile((mLocaleDir + mLocale + ".json").data(), nullptr));
	if (localeData == nullptr) {
		localeData = static_cast<char*>(SDL_LoadFile((mLocaleDir + main + ".json").data(), nullptr));

		if (localeData == nullptr) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
					"LocaleManager.cpp: Failed to find/read locale shource %s: %s\n",
					(mLocaleDir + main + ".json").data(), SDL_GetError());

			// ERROR_BOX("Failed to find/read locale");

			throw std::runtime_error("LocaleManager.cpp: Failed to find/read locale");
		}
	}

	mLocaleData = nlohmann::json::parse(localeData);

	SDL_Log("LocalManager.cpp: Locale %s version %d", mLocale.data(), mLocaleData["version"].get<int>());

	SDL_Log("LocalManager.cpp: Successfully loaded locale %s", mLocale.data());
}

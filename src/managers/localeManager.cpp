#include "managers/localeManager.hpp"

#include "third_party/json.hpp"
#include "utils.hpp"

#include <__format/format_functions.h>
#include <stdexcept>
#include <string>
#include <string_view>

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
		if (loc[i]->country != 0) {
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

		SDL_free(loc);

		return;
	}

	SDL_free(loc);

	mLocale = "en-US";
	SDL_Log("\x1B[31mLocaleManager.cpp: Failed to find valid locale! Falling back to english.\033[0m");
	loadLocale();
}

std::u32string LocaleManager::utf8_to_utf32(const std::string_view& u8) {
	std::u32string out;

	int elem_len = 1;
	for (size_t i = 0; i < u8.size(); i += elem_len) {
		uint32_t tmp = (uint32_t)u8[i] & 0xff;
		if (tmp < 0x80UL) {
			elem_len = 1;
			out.push_back(u8[i]);
		} else if (tmp < 0xe0UL) {
			elem_len = 2;
			out.push_back(((u8[i] & 0x1f) << 6) | (u8[i + 1] & 0x3f));
		} else if (tmp < 0xf0UL) {
			elem_len = 3;
			out.push_back(((u8[i] & 0xf) << 12) | ((u8[i + 1] & 0x3f) << 6) | (u8[i + 2] & 0x3f));
		} else if (tmp < 0xf8UL) {
			elem_len = 4;
			out.push_back(((u8[i] & 0x7) << 18) | ((u8[i + 1] & 0x3f) << 12) | ((u8[i + 2] & 0x3f) << 6) |
				      (u8[i + 3] & 0x3f));
		} else {
			throw std::runtime_error("LocaleManager.cpp: Error! couldn't convert utf8 to utf32");
		}
	}

	return true;
}

std::u32string LocaleManager::get(std::string_view id) {
	if (!mLocaleData.contains(id)) {
		SDL_Log("\x1B[31mLocaleManager.cpp: Error! Unknown id %s\033[0m", id.data());

#ifdef DEBUG
		throw std::runtime_error("LocaleManager.cpp: Unknown id!");
#endif

		return U"";
	}

	std::u32string result;
	utf8to32(s.begin(), s.end(), std::back_inserter(result));
	return result;
}

void LocaleManager::loadLocale() {
	SDL_Log("LocaleManager.cpp: Loading %s", mLocale.data());

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

	SDL_free(localeData);

	SDL_Log("LocaleManager.cpp: Found locale %s version %d", mLocale.data(), mLocaleData["version"].get<int>());

	SDL_Log("LocaleManager.cpp: Successfully loaded locale %s", mLocale.data());
}

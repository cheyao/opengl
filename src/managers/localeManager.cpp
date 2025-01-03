#include "managers/localeManager.hpp"

#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/error/en.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <version>

// TODO: Handle SDL_EVENT_LOCALE_CHANGED
LocaleManager::LocaleManager() : mLocaleDir(getBasePath() + "assets/strings/"), mLocaleDataS(nullptr, SDL_free) {
	int c = 0;
	SDL_Locale** const loc = SDL_GetPreferredLocales(&c);
	std::string locList = "";
	for (int i = 0; i < c; ++i) {
		locList += loc[i]->language;

		if (loc[i]->language != nullptr) {
			locList += "-";
			locList += loc[i]->country;
		}

		locList += " ";
	}

	SDL_Log("LocaleManager.cpp: Found locales %s", locList.data());

	for (int i = 0; i < c; ++i) {
		mLocale = loc[i]->language;
		if (loc[i]->country != nullptr) {
			mLocale.append("-");
			mLocale.append(loc[i]->country);
		}

		SDL_Log("LocaleManager.cpp: Tring to load locale %s", mLocale.data());

		try {
			loadLocale();
		} catch (const std::runtime_error& error) {
			SDL_Log("\033[33mLocaleManager.cpp: Failed to load locale %s: %s\033[0m", mLocale.data(),
				error.what());

			continue;
		}

		SDL_Log("\033[32mLocaleManager.cpp: Successfully loaded system locale %s\033[0m", mLocale.data());

		SDL_free(loc);

		return;
	}

	SDL_free(loc);

	mLocale = "en-US";
	SDL_Log("\033[31mLocaleManager.cpp: Failed to find valid locale! Falling back to en-US.\033[0m");
	loadLocale();
}

std::u32string LocaleManager::U8toU32(const std::string_view& u8) const {
	std::u32string out;

	int word_size = 1;
	for (std::size_t i = 0; i < u8.size(); i += word_size) {
		std::uint32_t tmp = static_cast<std::uint32_t>(u8[i]) & 0xff;

		if (tmp < 0x80UL) {
			word_size = 1;
			out.push_back(u8[i]);
		} else if (tmp < 0xe0UL) {
			word_size = 2;
			out.push_back(((u8[i] & 0x1f) << 6) | (u8[i + 1] & 0x3f));
		} else if (tmp < 0xf0UL) {
			word_size = 3;
			out.push_back(((u8[i] & 0xf) << 12) | ((u8[i + 1] & 0x3f) << 6) | (u8[i + 2] & 0x3f));
		} else if (tmp < 0xf8UL) {
			word_size = 4;
			out.push_back(((u8[i] & 0x7) << 18) | ((u8[i + 1] & 0x3f) << 12) | ((u8[i + 2] & 0x3f) << 6) |
				      (u8[i + 3] & 0x3f));
		} else {
			throw std::runtime_error("LocaleManager.cpp: Error! couldn't convert utf8 to utf32");
		}
	}

	return out;
}

std::u32string LocaleManager::get(const std::string_view& id) const {
	if (id[0] == '!') {
		return U8toU32(&id[1]);
	}

	if (!mLocaleData.HasMember(id.data())) {
		SDL_Log("\x1B[31mLocaleManager.cpp: Error! Unknown id %s\033[0m", id.data());

#ifdef DEBUG
		throw std::runtime_error("LocaleManager.cpp: Unknown id!");
#endif

		return U"NAN";
	}

	// Maybe https://wiki.libsdl.org/SDL3/SDL_iconv
	return U8toU32(mLocaleData[id.data()].GetString());
}

void LocaleManager::loadLocale() {
	SDL_Log("LocaleManager.cpp: Loading %s", mLocale.data());

	std::string main;

#ifdef __cpp_lib_string_contains
	if (mLocale.contains('-')) {
#else
	if (mLocale.find('-') != std::string::npos) {
#endif
		const std::size_t pos = mLocale.find('-');
		main = mLocale.substr(0, pos);
	} else {
		main = mLocale;
	}

	// Prefer specialized locale to generalized one
	mLocaleDataS.reset(static_cast<char*>(loadFile((mLocaleDir + mLocale + ".json").data(), nullptr)));

	if (!mLocaleDataS) {
		mLocaleDataS.reset(static_cast<char*>(loadFile((mLocaleDir + main + ".json").data(), nullptr)));

		if (!mLocaleDataS) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
					"LocaleManager.cpp: Failed to find/read locale shource %s: %s\n",
					(mLocaleDir + mLocale + ".json").data(), SDL_GetError());

			throw std::runtime_error("LocaleManager.cpp: Failed to find/read locale");
		}
	}

	if (mLocaleData.ParseInsitu(mLocaleDataS.get()).HasParseError()) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "\033[31mFailed to parse json (offset %u): %s\033[0m",
				(unsigned)mLocaleData.GetErrorOffset(),
				rapidjson::GetParseError_En(mLocaleData.GetParseError()));
		ERROR_BOX("Failed to load save file");
		ERROR_BOX("Failed to read locale, reinstall assets");

		throw std::runtime_error("StorageManager.cpp: Failed to parse json");
	}

	SDL_Log("LocaleManager.cpp: Found locale %s version %d", mLocale.data(), mLocaleData["version"].GetInt());

	SDL_Log("LocaleManager.cpp: Successfully loaded locale %s", mLocale.data());
}

#pragma once

#include "third_party/json.hpp"

#include <string>

class LocaleManager {
      public:
	LocaleManager(const std::string& path);
	LocaleManager(LocaleManager&&) = delete;
	LocaleManager(const LocaleManager&) = delete;
	LocaleManager& operator=(LocaleManager&&) = delete;
	LocaleManager& operator=(const LocaleManager&) = delete;
	~LocaleManager() = default;

	void changeLocale(std::string_view locale) {
		mLocale = locale;
		loadLocale();
	}
	std::u32string get(std::string_view id);

      private:
	static std::u32string utf8_to_utf32(const std::string_view& u8);
	void loadLocale();

	const std::string mLocaleDir;
	std::string mLocale;

	nlohmann::json mLocaleData;
};

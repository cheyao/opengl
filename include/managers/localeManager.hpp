#pragma once

#include "third_party/json.hpp"

#include <string>
#include <codecvt>

class LocaleManager {
      public:
	LocaleManager(const std::string& path);
	LocaleManager(LocaleManager&&) = delete;
	LocaleManager(const LocaleManager&) = delete;
	LocaleManager& operator=(LocaleManager&&) = delete;
	LocaleManager& operator=(const LocaleManager&) = delete;
	~LocaleManager();

	void changeLocale(std::string_view locale) { mLocale = locale; loadLocale();}
	std::u32string get(std::string_view id);

      private:
	void loadLocale();

	const std::string mLocaleDir;
	std::string mLocale;

	nlohmann::json mLocaleData;

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> mConverter;
};

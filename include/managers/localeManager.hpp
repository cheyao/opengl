#pragma once

#include "third_party/json.hpp"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <codecvt>
#include <string>

class LocaleManager {
      public:
	LocaleManager(const std::string& path);
	LocaleManager(LocaleManager&&) = delete;
	LocaleManager(const LocaleManager&) = delete;
	LocaleManager& operator=(LocaleManager&&) = delete;
	LocaleManager& operator=(const LocaleManager&) = delete;
	~LocaleManager();

	void changeLocale(std::string_view locale) {
		mLocale = locale;
		loadLocale();
	}
	std::u32string get(std::string_view id);

      private:
	void loadLocale();

	const std::string mLocaleDir;
	std::string mLocale;

	nlohmann::json mLocaleData;

	// FIXME: Decaprated
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> mConverter;
};

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

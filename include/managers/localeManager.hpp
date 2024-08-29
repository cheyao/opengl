#pragma once

#include "third_party/json.hpp"

#include <string>
#include <string_view>

class LocaleManager {
      public:
	LocaleManager(const std::string& path);
	LocaleManager(LocaleManager&&) = delete;
	LocaleManager(const LocaleManager&) = delete;
	LocaleManager& operator=(LocaleManager&&) = delete;
	LocaleManager& operator=(const LocaleManager&) = delete;
	~LocaleManager() = default;

	void changeLocale(const std::string_view& locale) {
		mLocale = locale;
		loadLocale();
	}
	std::u32string get(const std::string_view& id) const;

      private:
	std::u32string U8toU32(const std::string_view& u8) const;
	void loadLocale();

	const std::string mLocaleDir;
	std::string mLocale;

	nlohmann::json mLocaleData;
};

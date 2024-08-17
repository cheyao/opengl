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
	~LocaleManager();

      private:
	const std::string mLocaleDir;
	std::string mLocale;

	nlohmann::json mLocalData;
};

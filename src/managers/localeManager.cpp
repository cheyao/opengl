#include "managers/localeManager.hpp"

#include "third_party/json.hpp"
#include "utils.hpp"

#include <string>

LocaleManager::LocaleManager(const std::string& path)
	: mLocaleDir(path + "assets" SEPARATOR "textures" SEPARATOR), mLocale("en") {}


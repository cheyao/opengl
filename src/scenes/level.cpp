#include "scenes/level.hpp"

#include "third_party/json.hpp"

void Level::create() {}

void Level::load([[maybe_unused]] const nlohmann::json data) {}

nlohmann::json Level::save() { return nlohmann::json(); }

#include "scenes/level.hpp"

#include "game.hpp"
#include "third_party/json.hpp"

Level::Level(const std::string name) : mName(name) {}

void Level::create() {}

void Level::load([[maybe_unused]] const nlohmann::json data, [[maybe_unused]] Game* game) {}

nlohmann::json Level::save([[maybe_unused]] Game* game) { return nlohmann::json(); }

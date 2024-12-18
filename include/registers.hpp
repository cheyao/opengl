#pragma once

#include "components.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace Components {
enum class Item : std::uint64_t { AIR = 0, GRASS_BLOCK, STONE, OAK_LOG };
}

namespace registers {
extern const std::unordered_map<Components::Item, std::string> TEXTURES;
extern const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES;
} // namespace registers

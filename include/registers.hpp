#pragma once

#include "components.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace registers {
extern const std::unordered_map<Components::Item, std::string> TEXTURES;
extern const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES;
} // namespace registers

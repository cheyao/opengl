#pragma once

#include "components.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Components {
enum class Item : std::uint64_t { AIR = 0, GRASS_BLOCK, STONE, OAK_LOG, OAK_LEAVES };
}

namespace registers {
extern const std::unordered_map<Components::Item, std::string> TEXTURES;
extern const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES;
extern const std::vector<std::pair<float, std::vector<std::pair<Components::Item, Eigen::Vector2i>>>>
	SURFACE_STRUCTURES;
extern const std::unordered_map<Components::Item, std::vector<std::pair<float, Components::Item>>> LOOT_TABLES;
} // namespace registers

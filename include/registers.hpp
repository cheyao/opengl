#pragma once

#include "components.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Components {
// Only append! Don't change order to conserve backwards compatability
enum class Item : std::uint64_t { AIR = 0, GRASS_BLOCK, STONE, OAK_LOG, OAK_LEAVES, OAK_PLANKS };
} // namespace Components

namespace registers {
extern const std::unordered_map<Components::Item, std::string> TEXTURES;
extern const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES;
extern const std::vector<std::pair<float, std::vector<std::pair<Components::Item, Eigen::Vector2i>>>>
	SURFACE_STRUCTURES;
extern const std::unordered_map<Components::Item, std::vector<std::pair<float, Components::Item>>> LOOT_TABLES;
extern const std::vector<
	std::tuple<std::uint64_t, std::vector<Components::Item>, std::pair<std::uint64_t, Components::Item>>>
	CRAFTING_RECIPIES;
} // namespace registers

#pragma once

#include "components.hpp"
#include "screens/screen.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Components {
// Only append! Don't change order to conserve backwards compatability
enum class Item : std::uint64_t;

consteval inline static auto AIR() { return static_cast<Item>(0); }
} // namespace Components

namespace registers {
enum class MiningSystem;

extern const std::unordered_map<Components::Item, std::string> TEXTURES;
extern const std::unordered_map<Components::Item, std::pair<int, std::uint64_t>> BREAK_TIMES;
extern const std::unordered_map<Components::Item, int> MINING_LEVEL;
extern const std::unordered_map<Components::Item, registers::MiningSystem> MINING_SYSTEM;

extern const std::unordered_map<Components::Item, double> BURNING_TIME;
extern const std::unordered_map<Components::Item, std::pair<double, Components::Item>> SMELTING_RECIPIE;

extern const std::vector<std::pair<float, std::vector<std::pair<Components::Item, Eigen::Vector2i>>>>
	SURFACE_STRUCTURES;
extern const std::unordered_map<Components::Item, std::vector<std::pair<float, Components::Item>>> LOOT_TABLES;
extern const std::vector<std::tuple<std::pair<std::uint64_t, std::uint64_t>, std::vector<Components::Item>,
				    std::pair<std::uint64_t, Components::Item>>>
	CRAFTING_RECIPIES;
extern const std::unordered_map<Components::Item, class Screen*(*)(void)> CLICKABLES;
extern const std::vector<std::string> BACKGROUND_SOUNDS;

extern const std::unordered_map<Components::Item, std::pair<Eigen::Vector2f, Eigen::Vector2f>> COLLISION_BOXES;
} // namespace registers

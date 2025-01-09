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

// item to texture path
extern const std::unordered_map<Components::Item, std::string> TEXTURES;

// Map to the time to break the block: {tool and time in ticks}
extern const std::unordered_map<Components::Item, std::pair<int, std::uint64_t>> BREAK_TIMES;

// Notes the mining level of tools (minimal tool grade)
extern const std::unordered_map<Components::Item, int> MINING_LEVEL;

// Mining register, if a block is pickaxe. axe shovel etc. Not present = no preference
extern const std::unordered_map<Components::Item, registers::MiningSystem> MINING_SYSTEM;

// Burning time register, map of item to smelting time in s
extern const std::unordered_map<Components::Item, double> BURNING_TIME;

// Smelting recipies, map from item to {smelting time in s, output item}
extern const std::unordered_map<Components::Item, std::pair<double, Components::Item>> SMELTING_RECIPIE;

// Structures, vector of {chance, vector of blocks to spawn: {block, vector}}
extern const std::vector<std::pair<float, std::vector<std::pair<Components::Item, Eigen::Vector2i>>>>
	SURFACE_STRUCTURES;

// Vector of loot, map from item to vector of items with {chance, item}
extern const std::unordered_map<Components::Item, std::vector<std::pair<float, Components::Item>>> LOOT_TABLES;

// Crafting recipies,
// Vector of tuples with feilds <recipie size (axb, 0x0 for unordered crafting), items in the upside-down way if
// ordered, and {count, output}
extern const std::vector<std::tuple<std::pair<std::uint64_t, std::uint64_t>, std::vector<Components::Item>,
				    std::pair<std::uint64_t, Components::Item>>>
	CRAFTING_RECIPIES;

// Ascociation between item and functions for clickable items
extern const std::unordered_map<Components::Item, class Screen* (*)(void)> CLICKABLES;

// String of sounds
extern const std::vector<std::string> BACKGROUND_SOUNDS;

// Map of item to pair of offset and size
// Size 0x0 is no collision box
extern const std::unordered_map<Components::Item, std::pair<Eigen::Vector2f, Eigen::Vector2f>> COLLISION_BOXES;

// Vector of {Ore type and count}
extern const std::vector<std::pair<Components::Item, std::uint64_t>> VEINS;
} // namespace registers

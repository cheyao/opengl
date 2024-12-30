#include "registers.hpp"

#include "components.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace registers {

using namespace Components;

const std::unordered_map<Components::Item, std::string> TEXTURES = {
	{Item::GRASS_BLOCK, "blocks/grass-block.png"}, {Item::STONE, "blocks/stone.png"},
	{Item::OAK_LOG, "blocks/oak-log.png"},	       {Item::OAK_LEAVES, "blocks/oak-leaves.png"},
	{Item::OAK_PLANKS, "blocks/oak-planks.png"},
};

const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES = {
	{Item::AIR, 0},	     {Item::GRASS_BLOCK, 20}, {Item::STONE, 80},
	{Item::OAK_LOG, 60}, {Item::OAK_LEAVES, 20},  {Item::OAK_PLANKS, 60}};

const std::vector<std::pair<float, std::vector<std::pair<Components::Item, Eigen::Vector2i>>>> SURFACE_STRUCTURES = {
	{0.08,
	 {
		 // Main Bark
		 {Item::OAK_LOG, {0, 0}},
		 {Item::OAK_LOG, {0, 1}},
		 {Item::OAK_LOG, {0, 2}},
		 {Item::OAK_LOG, {0, 3}},
		 // Leafs
		 {Item::OAK_LEAVES, {0, 5}},
		 {Item::OAK_LEAVES, {1, 5}},
		 {Item::OAK_LEAVES, {-1, 5}},

		 {Item::OAK_LEAVES, {-2, 4}},
		 {Item::OAK_LEAVES, {-1, 4}},
		 {Item::OAK_LEAVES, {0, 4}},
		 {Item::OAK_LEAVES, {1, 4}},
		 {Item::OAK_LEAVES, {2, 4}},

		 {Item::OAK_LEAVES, {-2, 3}},
		 {Item::OAK_LEAVES, {-1, 3}},
		 {Item::OAK_LEAVES, {1, 3}},
		 {Item::OAK_LEAVES, {2, 3}},
	 }},
};

const std::unordered_map<Components::Item, std::vector<std::pair<float, Components::Item>>> LOOT_TABLES = {
	{Item::OAK_LEAVES,
	 {
		 {0.2f, Item::OAK_PLANKS},
	 }},
};

const std::vector<std::tuple<std::uint64_t, std::vector<Components::Item>, std::pair<std::uint64_t, Components::Item>>>
	CRAFTING_RECIPIES = {{}, {0, {Item::OAK_LOG}, {4, Item::OAK_PLANKS}}};

} // namespace registers

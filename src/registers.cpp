#include "registers.hpp"

#include "components.hpp"
#include "components/crafting.hpp"
#include "components/furnace.hpp"
#include "items.hpp"
#include "screens/screen.hpp"
#include "third_party/Eigen/Core"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

namespace registers {

using namespace Components;

enum class MiningSystem { PICKAXE, AXE, SHOVEL, HOE, SHEARS };

const std::unordered_map<Components::Item, std::string> TEXTURES = {
	{Item::GRASS_BLOCK, "blocks/grass-block.png"},
	{Item::DIRT, "blocks/dirt.png"},
	{Item::STONE, "blocks/stone.png"},
	{Item::OAK_LOG, "blocks/oak-log.png"},
	{Item::OAK_LEAVES, "blocks/oak-leaves.png"},
	{Item::OAK_PLANKS, "blocks/oak-planks.png"},
	{Item::STICK, "items/stick.png"},
	{Item::CRAFTING_TABLE, "blocks/crafting-table.png"},
	{Item::WOODEN_SHOVEL, "items/wooden-shovel.png"},
	{Item::WOODEN_AXE, "items/wooden-axe.png"},
	{Item::WOODEN_HOE, "items/wooden-hoe.png"},
	{Item::WOODEN_PICKAXE, "items/wooden-pickaxe.png"},
	{Item::STONE_SHOVEL, "items/stone-shovel.png"},
	{Item::STONE_AXE, "items/stone-axe.png"},
	{Item::STONE_HOE, "items/stone-hoe.png"},
	{Item::STONE_PICKAXE, "items/stone-pickaxe.png"},
	{Item::COBBLESTONE, "blocks/cobblestone.png"},
	{Item::FURNACE, "blocks/furnace.png"},
	{Item::APPLE, "items/apple.png"},
	{Item::TORCH, "blocks/torch.png"},
	{Item::CAMPFIRE, "blocks/campfire.png"},
	{Item::CHARCOAL, "items/charcoal.png"},
};

const std::unordered_map<Components::Item, std::pair<int, std::uint64_t>> BREAK_TIMES = {
	{Item::AIR, {0, 0}},	     {Item::GRASS_BLOCK, {0, 40}},    {Item::DIRT, {0, 40}},
	{Item::STONE, {1, 80}},	     {Item::OAK_LOG, {0, 50}},	      {Item::OAK_LEAVES, {0, 20}},
	{Item::OAK_PLANKS, {0, 60}}, {Item::CRAFTING_TABLE, {0, 50}}, {Item::COBBLESTONE, {1, 80}},
	{Item::FURNACE, {1, 80}},    {Item::CAMPFIRE, {0, 50}},	      {Item::TORCH, {0, 2}},
};

// Will add one in the real calculation
// WOOD 1 STONE 3 IRON 5 diamond 7 neth 8 gold 11
extern const std::unordered_map<Components::Item, int> MINING_LEVEL = {
	{Item::WOODEN_PICKAXE, 1}, {Item::WOODEN_AXE, 1}, {Item::WOODEN_SHOVEL, 1}, {Item::WOODEN_HOE, 1},
	{Item::STONE_PICKAXE, 3},  {Item::STONE_AXE, 3},  {Item::STONE_SHOVEL, 3},  {Item::STONE_HOE, 3},
};
extern const std::unordered_map<Components::Item, registers::MiningSystem> MINING_SYSTEM = {
	{Item::GRASS_BLOCK, MiningSystem::SHOVEL},     {Item::DIRT, MiningSystem::SHOVEL},
	{Item::STONE, MiningSystem::PICKAXE},	       {Item::OAK_LOG, MiningSystem::AXE},
	{Item::OAK_LEAVES, MiningSystem::HOE},	       {Item::OAK_PLANKS, MiningSystem::AXE},
	{Item::CRAFTING_TABLE, MiningSystem::AXE},     {Item::WOODEN_SHOVEL, MiningSystem::SHOVEL},
	{Item::WOODEN_AXE, MiningSystem::AXE},	       {Item::WOODEN_HOE, MiningSystem::HOE},
	{Item::WOODEN_PICKAXE, MiningSystem::PICKAXE}, {Item::STONE_SHOVEL, MiningSystem::SHOVEL},
	{Item::STONE_AXE, MiningSystem::AXE},	       {Item::STONE_HOE, MiningSystem::HOE},
	{Item::STONE_PICKAXE, MiningSystem::PICKAXE},  {Item::COBBLESTONE, MiningSystem::PICKAXE},
	{Item::FURNACE, MiningSystem::PICKAXE},	       {Item::CAMPFIRE, MiningSystem::AXE}};

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
		 {0.2f, Item::STICK},
		 {0.1f, Item::APPLE},
	 }},
	{Item::GRASS_BLOCK,
	 {
		 {1.1f, Item::DIRT},
	 }},
	{Item::STONE,
	 {
		 {1.1f, Item::COBBLESTONE},
	 }},
	{Item::CAMPFIRE,
	 {
		 {1.1f, Item::CHARCOAL},
		 {1.1f, Item::CHARCOAL},
	 }},
};

// NOLINTBEGIN
// ITEMS ARE IN REVERSE!!!
const std::vector<std::tuple<std::pair<std::uint64_t, std::uint64_t>, std::vector<Components::Item>,
			     std::pair<std::uint64_t, Components::Item>>>
	CRAFTING_RECIPIES = {
		{/* KEEP THIS! 0th recipie represents no recipie*/},
		{{0, 0}, {Item::OAK_LOG}, {4, Item::OAK_PLANKS}},
		{{1, 2}, {Item::OAK_PLANKS, Item::OAK_PLANKS}, {4, Item::STICK}},
		{{2, 2},
		 {Item::OAK_PLANKS, Item::OAK_PLANKS, Item::OAK_PLANKS, Item::OAK_PLANKS},
		 {1, Item::CRAFTING_TABLE}},
		{{3, 3},
		 {
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
			 Item::OAK_PLANKS,
			 Item::OAK_PLANKS,
			 Item::OAK_PLANKS,
		 },
		 {1, Item::WOODEN_PICKAXE}},
		{{2, 3},
		 {
			 Item::AIR,
			 Item::STICK,
			 Item::OAK_PLANKS,
			 Item::STICK,
			 Item::OAK_PLANKS,
			 Item::OAK_PLANKS,
		 },
		 {1, Item::WOODEN_AXE}},
		{{1, 3},
		 {
			 Item::STICK,
			 Item::STICK,
			 Item::OAK_PLANKS,
		 },
		 {1, Item::WOODEN_SHOVEL}},
		{{2, 3},
		 {
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
			 Item::STICK,
			 Item::OAK_PLANKS,
			 Item::OAK_PLANKS,
		 },
		 {1, Item::WOODEN_HOE}},
		{{3, 3},
		 {
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
		 },
		 {1, Item::STONE_PICKAXE}},
		{{2, 3},
		 {
			 Item::AIR,
			 Item::STICK,
			 Item::COBBLESTONE,
			 Item::STICK,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
		 },
		 {1, Item::STONE_AXE}},
		{{1, 3},
		 {
			 Item::STICK,
			 Item::STICK,
			 Item::COBBLESTONE,
		 },
		 {1, Item::STONE_SHOVEL}},
		{{2, 3},
		 {
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
			 Item::STICK,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
		 },
		 {1, Item::STONE_HOE}},
		{{3, 3},
		 {
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
			 Item::AIR,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
			 Item::COBBLESTONE,
		 },
		 {1, Item::FURNACE}},
		{{3, 3},
		 {
			 Item::OAK_LOG,
			 Item::OAK_LOG,
			 Item::OAK_LOG,
			 Item::STICK,
			 Item::CHARCOAL,
			 Item::STICK,
			 Item::AIR,
			 Item::STICK,
			 Item::AIR,
		 },
		 {1, Item::CAMPFIRE}},
		{{1, 2},
		 {
			 Item::STICK,
			 Item::CHARCOAL,
		 },
		 {1, Item::TORCH}},
};
// NOLINTEND

// Time in seconds
const std::unordered_map<Components::Item, double> BURNING_TIME = {
	{Item::OAK_LOG, 16},	   {Item::OAK_PLANKS, 4}, {Item::STICK, 2},	    {Item::WOODEN_AXE, 8},
	{Item::WOODEN_PICKAXE, 8}, {Item::WOODEN_HOE, 8}, {Item::WOODEN_SHOVEL, 8}, {Item::CHARCOAL, 32},
};

// Time in seconds
const std::unordered_map<Components::Item, std::pair<double, Components::Item>> SMELTING_RECIPIE = {
	{Item::COBBLESTONE, {8, Item::STONE}},
	{Item::OAK_LOG, {8, Item::CHARCOAL}},
};

template <typename T, typename... Args> T* staticHelper(Args&&... args) {
	static T t(std::forward<Args>(args)...);
	return &t;
};

const std::unordered_map<Components::Item, Screen* (*)(void)> CLICKABLES = {
	{Item::CRAFTING_TABLE, [] -> Screen* { return staticHelper<CraftingInventory>(crafting_table_t()); }},
	{Item::FURNACE, [] -> Screen* { return staticHelper<FurnaceInventory>(furnace_t()); }},
};

const std::vector<std::string> BACKGROUND_SOUNDS = {
	"sweden.wav",
	"subwoofer_lullaby.wav",
};

constexpr inline auto size = 16 * 7;
extern const std::unordered_map<Components::Item, std::pair<Eigen::Vector2f, Eigen::Vector2f>> COLLISION_BOXES = {
	{Item::CAMPFIRE, {Eigen::Vector2f(0, 0), Eigen::Vector2f(size, size / 2)}},
	{Item::TORCH, {Eigen::Vector2f(0, 0), Eigen::Vector2f(0, 0)}},
};
} // namespace registers

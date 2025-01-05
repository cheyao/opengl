#pragma once

#include <cstdint>

namespace Components {
enum class Item : std::uint64_t {
	AIR = 0,
	GRASS_BLOCK,
	STONE,
	OAK_LOG,
	OAK_LEAVES,
	OAK_PLANKS,
	STICK,
	DIRT,
	CRAFTING_TABLE,
	WOODEN_PICKAXE,
	WOODEN_AXE,
	WOODEN_SHOVEL,
	WOODEN_HOE,
	STONE_PICKAXE,
	STONE_AXE,
	STONE_SHOVEL,
	STONE_HOE,
	COBBLESTONE,
	FURNACE,
	APPLE,
	CHARCOAL,
	TORCH,
	CAMPFIRE
};
} // namespace Components

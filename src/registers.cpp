#include "registers.hpp"

#include "components.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace registers {
const std::unordered_map<Components::Item, std::string> TEXTURES = {
	{Components::Item::GRASS_BLOCK, "blocks/grass-block.png"},
	{Components::Item::STONE, "blocks/stone.png"},
};

const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES = {
	{Components::Item::AIR, 0},
	{Components::Item::GRASS_BLOCK, 20},
	{Components::Item::STONE, 80},
};
} // namespace registers

#include "registers.hpp"

#include "components.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace registers {
const std::unordered_map<Components::Item, std::string> TEXTURES = {
	{Components::Item::GRASS_BLOCK, "blocks/grass-block.png"},
	{Components::Item::STONE, "blocks/stone.png"},
	{Components::Item::OAK_LOG, "blocks/oak-log.png"},
};

const std::unordered_map<Components::Item, std::uint64_t> BREAK_TIMES = {
	{Components::Item::AIR, 0},
	{Components::Item::GRASS_BLOCK, 20},
	{Components::Item::STONE, 80},
	{Components::Item::OAK_LOG, 60},
};

const std::vector<std::pair<float, std::vector<std::pair<Components::Item, Eigen::Vector2i>>>> SURFACE_STRUCTURES = {
	{0.1, {{Components::Item::OAK_LOG, {0, 0}}, {Components::Item::OAK_LOG, {0, 1}}}}};
} // namespace registers

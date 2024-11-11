#include "components/inventory.hpp"

#include "screens/screen.hpp"
#include "third_party/json.hpp"

#include <cstddef>

Inventory::Inventory(class Game* game, const std::size_t size) : Screen(game), mSize(size), mVector(size) {}
Inventory::Inventory(class Game* game, const nlohmann::json& contents)
	: Screen(game), mSize(contents[SIZE_KEY]), mVector(contents[CONTENTS_KEY]) {}

nlohmann::json Inventory::save() {
	nlohmann::json contents;

	contents[SIZE_KEY] = mSize;
	contents[CONTENTS_KEY] = mVector;

	return contents;
}

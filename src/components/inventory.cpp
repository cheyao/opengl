#include "components/inventory.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"
#include "screens/screen.hpp"
#include "systems/UISystem.hpp"
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

void Inventory::update(class Scene* scene, float delta) {(void)scene;(void)delta;}
void Inventory::draw(class Scene* scene) {(void)scene;}

void Inventory::close() { mGame->getSystemManager()->getUISystem()->pop(); }

#include "components/inventory.hpp"

#include "game.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "screens/screen.hpp"
#include "systems/UISystem.hpp"
#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include <SDL3/SDL.h>
#include <cstddef>

Inventory::Inventory(class Game* game, const std::size_t size, EntityID entity)
	: Screen(game), mEntity(entity), mSize(size), mVector(size) {}
Inventory::Inventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Screen(game), mEntity(entity), mSize(contents[SIZE_KEY]), mVector(contents[CONTENTS_KEY]) {}

nlohmann::json Inventory::save() {
	nlohmann::json contents;

	contents[SIZE_KEY] = mSize;
	contents[CONTENTS_KEY] = mVector;

	return contents;
}

void Inventory::update(class Scene* scene, float) {
	// Handle mouse and keys
	const auto keystate = mGame->getKeystate();

	// Quit menu
	if (keystate[SDL_SCANCODE_ESCAPE]) {
		mGame->getSystemManager()->getUISystem()->pop();
	}

	(void)scene;
}

void Inventory::draw(class Scene*) {
	constexpr const static auto INVENTORY_TEXTURE_WIDTH = 176.0f;
	constexpr const static auto INVENTORY_TEXTURE_HEIGHT = 166.0f;
	constexpr const static auto INVENTORY_SPRITE_FILE = "ui/inventory.png";

	// Draw the inventory
	auto* systemManager = mGame->getSystemManager();
	Shader* shader = systemManager->getShader("ui.vert", "ui.frag");
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	Texture* texture = systemManager->getTexture(INVENTORY_SPRITE_FILE);
	auto mesh = systemManager->getUISystem()->getMesh();

	float x, y;
	float ox, oy;

	if (dimensions.x() <= dimensions.y()) {
		x = dimensions.x() / 4 * 3;
		y = x / INVENTORY_TEXTURE_WIDTH * INVENTORY_TEXTURE_HEIGHT;
		ox = x / 6;
		oy = (dimensions.y() - y) / 2;
	} else {
		y = dimensions.y() / 4 * 3;
		x = y / INVENTORY_TEXTURE_HEIGHT * INVENTORY_TEXTURE_WIDTH;
		ox = (dimensions.x() - x) / 2;
		oy = y / 6;
	}

	shader->activate();

	shader->set("texture_diffuse"_u, 0);
	shader->set("offset"_u, ox, oy);
	shader->set("size"_u, x, y);

	texture->activate(0);

	mesh->draw(shader);
}

void Inventory::close() { mGame->getSystemManager()->getUISystem()->pop(); }

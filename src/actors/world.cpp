#include "actors/world.hpp"

#include "actors/sun.hpp"
#include "components/meshComponent.hpp"
#include "components/modelComponent.hpp"
#include "game.hpp"
#include "third_party/glad/glad.h"

World::World(class Game* game) : Actor(game) {
	/*
	const std::vector<Vertex> vertices = {
		{{-0.5f, +0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Top left
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bot left
		{{+0.5f, +0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // Top right
		{{+0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Bot right
	};
	const std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2};
	const std::vector<std::pair<Texture*, TextureType>> textures = {
		std::make_pair(this->getGame()->getTexture("windows.png"), TextureType::DIFFUSE)};

	MeshComponent* const window = new MeshComponent(this, vertices, indices, textures, 300);
	window->setVert("common.vert");
	window->setFrag("window.frag");
	*/

	return;
	ModelComponent* const model =
		new ModelComponent(this, getGame()->fullPath("models" SEPARATOR "earth.blend"));
	model->setVert("common.vert");
	model->setFrag("backpack.frag");
	model->setGeom("backpack.geom");
	model->addTexture(
		std::make_pair(this->getGame()->getTexture("land.png"), TextureType::DIFFUSE));

	// TODO: constexpr? Should be able to do this in c++23 duh
	const std::vector<Vertex> verticesBox = {
		{{-1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0 FLD
		{{+1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1 FRD
		{{-1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 2 FLU
		{{+1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3 FRU
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4 BLD
		{{+1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 5 BRD
		{{-1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 6 BLU
		{{+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}  // 7 BRU
	};
	const std::vector<GLuint> indicesBox = {2, 6, 7, 2, 7, 3, // Top

											0, 5, 4, 0, 1, 5,  // Bottom
											1, 6, 2, 0, 4, 6,  // Left
											1, 3, 7, 1, 7, 5,  // Right
											0, 2, 3, 0, 3, 1,  // Front
											4, 7, 6, 4, 5, 7}; // Back
	const std::vector<std::pair<const Texture*, const TextureType>> texturesBox = {
		std::make_pair(this->getGame()->getTexture("nightsky"), TextureType::DIFFUSE)};

	MeshComponent* const box = new MeshComponent(this, verticesBox, indicesBox, texturesBox, 200);
	box->setVert("sky.vert");
	box->setFrag("sky.frag");

	setScale(5.0f);

	new Sun(game);
}

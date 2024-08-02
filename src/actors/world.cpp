#include "actors/world.hpp"

#include "components/meshComponent.hpp"
#include "components/modelComponent.hpp"
#include "game.hpp"
#include "third_party/glad/glad.h"

World::World(class Game* owner) : Actor(owner) {
	ModelComponent* const model =
		new ModelComponent(this, getGame()->fullPath("models" SEPARATOR "backpack.obj"));
	model->setVert("common.vert");
	model->setFrag("backpack.frag");

	const std::vector<Vertex> vertices = {
		{{-0.5f, +0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Top left
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bot left
		{{+0.5f, +0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // Top right
		{{+0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Bot right
	};
	const std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2};
	const std::vector<std::pair<Texture*, TextureType>> textures = {
		std::make_pair(this->getGame()->getTexture("windows.png"), TextureType::DIFFUSE)};

	MeshComponent* const window = new MeshComponent(this, vertices, indices, textures);
	window->setVert("common.vert");
	window->setFrag("window.frag");

	const std::vector<Vertex> verticesBox = {
		{{-1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0
		{{+1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1
		{{-1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 2
		{{+1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4
		{{+1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 5
		{{-1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 6
		{{+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}  // 7
	};
	const std::vector<GLuint> indicesBox = {// Top
											2, 6, 7, 2, 3, 7,

											// Bottom
											0, 4, 5, 0, 1, 5,

											// Left
											0, 2, 6, 0, 4, 6,

											// Right
											1, 3, 7, 1, 5, 7,

											// Front
											0, 2, 3, 0, 1, 3,

											// Back
											4, 6, 7, 4, 5, 7};
	const std::vector<std::pair<Texture*, TextureType>> texturesBox = {
		std::make_pair(this->getGame()->getTexture("skybox"), TextureType::DIFFUSE)};

	MeshComponent* const box = new MeshComponent(this, verticesBox, indicesBox, texturesBox, 5);
	box->setVert("cubemap.vert");
	box->setFrag("cubemap.frag");
}

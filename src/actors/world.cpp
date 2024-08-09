#include "actors/world.hpp"

#include "actors/sun.hpp"
#include "components/meshComponent.hpp"
#include "components/modelComponent.hpp"
#include "game.hpp"
#include "third_party/glad/glad.h"

World::World(class Game* game) : Actor(game) {
	ModelComponent* const model = new ModelComponent(this, getGame()->fullPath("models" SEPARATOR "earth.blend"));
	model->setVert("common.vert");
	model->setFrag("backpack.frag");
	model->addTexture(std::make_pair(this->getGame()->getTexture("land.png"), TextureType::DIFFUSE));

	const std::vector<const float> verticesBox = {
		-1.0f, -1.0f, +1.0f, // 0 FLD
		+1.0f, -1.0f, +1.0f, // 1 FRD
		-1.0f, +1.0f, +1.0f, // 2 FLU
		+1.0f, +1.0f, +1.0f, // 3 FRU
		-1.0f, -1.0f, -1.0f, // 4 BLD
		+1.0f, -1.0f, -1.0f, // 5 BRD
		-1.0f, +1.0f, -1.0f, // 6 BLU
		+1.0f, +1.0f, -1.0f  // 7 BRU
	};
	const std::vector<const GLuint> indicesBox = {2, 6, 7, 2, 7, 3, // Top
						      0, 5, 4, 0, 1, 5, // Bottom
						      0, 2, 3, 0, 3, 1, // Front
						      4, 7, 6, 4, 5, 7, // Back
						      1, 3, 7, 1, 7, 5, // Right
						      0, 6, 2, 0, 4, 6}; // Left
	const std::vector<const std::pair<const Texture* const, const TextureType>> texturesBox = {
		std::make_pair(this->getGame()->getTexture("nightsky"), TextureType::DIFFUSE)};

	MeshComponent* const box = new MeshComponent(this, verticesBox, {}, {}, indicesBox, texturesBox, 200);
	box->setVert("sky.vert");
	box->setFrag("sky.frag");

	setScale(5.0f);
}

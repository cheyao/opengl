#include "actors/world.hpp"

#include "components/meshComponent.hpp"
#include "components/modelComponent.hpp"
#include "game.hpp"

World::World(class Game* owner) : Actor(owner) {
	ModelComponent* const model = new ModelComponent(this, getGame()->fullPath("models" SEPARATOR "backpack.obj"));
	model->setShader(this->getGame()->getShader("basic.vert", "basic.frag"));

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
	window->setShader(this->getGame()->getShader("basic.vert", "window.frag"));
}

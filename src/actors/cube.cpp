#include "actors/cube.hpp"

#include "components/modelComponent.hpp"
#include "game.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cstdlib>

Cube::Cube(class Game* owner) : Actor(owner) {
	const std::vector<Vertex> verticesCube = {
		{{-1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0 FLD
		{{+1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1 FRD
		{{-1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 2 FLU
		{{+1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3 FRU
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4 BLD
		{{+1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 5 BRD
		{{-1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 6 BLU
		{{+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}  // 7 BRU
	};
	const std::vector<GLuint> indicesCube = {2, 7, 6, 2, 3, 7,	// Top
											 0, 4, 5, 0, 5, 1,	// Bottom
											 0, 2, 6, 0, 6, 4,	// Left
											 1, 7, 3, 1, 5, 7,	// Right
											 0, 3, 2, 0, 1, 3,	// Front
											 4, 6, 7, 4, 7, 5}; // Back

	ModelComponent* const meteorites = new ModelComponent(
		this, this->getGame()->fullPath("models" SEPARATOR "rock.obj")); //, verticesCube, indicesCube, {}, 200);
	meteorites->setVert("cube.vert");
	meteorites->setFrag("cube.frag");

	constexpr const unsigned int amount = 5000;

	Eigen::Affine3f* modelMatrices = new Eigen::Affine3f[amount];

	float radius = 25.0f;
	float offset = 2.5f;
	for (unsigned int i = 0; i < amount; i++) {
		Eigen::Affine3f model = Eigen::Affine3f::Identity();

		const float angle = static_cast<float>(i) / static_cast<float>(amount) * 360.0f;
		Eigen::Vector3f translation;
		float displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		translation.x() = sin(angle) * radius + displacement;
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		translation.y() = displacement * 0.4f;
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		translation.z() = cos(angle) * radius + displacement;
		model.translate(translation);

		const float scale = (rand() % 20) / 100.0f + 0.05;
		model.scale(scale);

		const float rotAngle = (rand() % 360);
		model.rotate(Eigen::AngleAxisf(rotAngle, Eigen::Vector3f(0.4f, 0.6f, 0.8f).normalized()));

		modelMatrices[i] = model;
	}

	meteorites->addAttribArray(amount * sizeof(Eigen::Affine3f), &modelMatrices[0], []() {
		constexpr size_t vecSize = sizeof(Eigen::Vector4f);

		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vecSize,
							  reinterpret_cast<GLvoid*>(0 * vecSize));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vecSize,
							  reinterpret_cast<GLvoid*>(1 * vecSize));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vecSize,
							  reinterpret_cast<GLvoid*>(2 * vecSize));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vecSize,
							  reinterpret_cast<GLvoid*>(3 * vecSize));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	});

	meteorites->setDrawFunc(std::bind(glDrawElementsInstanced, std::placeholders::_1,
									  std::placeholders::_2, std::placeholders::_3,
									  std::placeholders::_4, amount));

	setScale(0.25);
}

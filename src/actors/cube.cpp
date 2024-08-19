#include "actors/cube.hpp"

#include "components/modelComponent.hpp"
#include "game.hpp"
#include "opengl/shader.hpp"
#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"

#include <SDL3/SDL.h>
#include <cstdlib>

#ifdef IMGUI
#include "imgui.h"
#endif

Cube::Cube(class Game* owner) : Actor(owner) {
	constexpr const GLuint ammount = 5000;
	ModelComponent* const meteorites =
		new ModelComponent(this, this->getGame()->fullPath("models" SEPARATOR "rock.obj"));
	mMeteorites = meteorites;
	meteorites->setShaders("cube.vert", "cube.frag");
	meteorites->setDrawFunc(std::bind(glDrawElementsInstanced, std::placeholders::_1, std::placeholders::_2,
					  std::placeholders::_3, std::placeholders::_4, ammount));

	Eigen::Affine3f* modelMatrices = new Eigen::Affine3f[ammount];

	constexpr const float radius = 25.0f;
	constexpr const float offset = 2.5f;
	for (unsigned int i = 0; i < ammount; ++i) {
		Eigen::Affine3f model = Eigen::Affine3f::Identity();

		const float angle = static_cast<float>(i) / static_cast<float>(ammount) * 360.0f;
		Eigen::Vector3f translation;

		float displacement;
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		translation.x() = sin(angle) * radius + displacement;
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		translation.y() = displacement * 0.4f + 5.0f;
		displacement = (rand() % static_cast<int>(2 * offset * 100)) / 100.0f - offset;
		translation.z() = cos(angle) * radius + displacement;

		model.translate(translation);

		const float scale = (rand() % 20) / 100.0f + 0.05;
		model.scale(scale);

		const float rotAngle = (rand() % 360);
		model.rotate(Eigen::AngleAxisf(rotAngle, Eigen::Vector3f(0.4f, 0.6f, 0.8f).normalized()));

		modelMatrices[i] = model;
	}

	static_assert(sizeof(Eigen::Affine3f) == 4 * sizeof(Eigen::Vector4f), "mat4 = 4 * vec4");
	meteorites->addAttribArray(ammount * sizeof(Eigen::Affine3f), &modelMatrices[0], []() {
		constexpr size_t vecSize = sizeof(Eigen::Vector4f);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, reinterpret_cast<GLvoid*>(0 * vecSize));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, reinterpret_cast<GLvoid*>(1 * vecSize));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, reinterpret_cast<GLvoid*>(2 * vecSize));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, reinterpret_cast<GLvoid*>(3 * vecSize));

		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	});

	delete[] modelMatrices;

	meteorites->addUniform([](Shader* shader) {
		const float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;

		shader->set("rotation", Eigen::Affine3f(Eigen::AngleAxisf(time / 3, Eigen::Vector3f::UnitY())));
	});

	setScale(0.25);
}

void Cube::updateActor([[maybe_unused]] float delta) {
#ifdef IMGUI
	static int ammount = 5000;
	
	ImGui::Begin("Cube");
	if (ImGui::SliderInt("Ammount", &ammount, 0, 20000)) {
		mMeteorites->setDrawFunc(std::bind(glDrawElementsInstanced, std::placeholders::_1, std::placeholders::_2,
						  std::placeholders::_3, std::placeholders::_4, ammount));
	}
	ImGui::End();
#endif
}

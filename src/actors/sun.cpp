#include "actors/sun.hpp"

#include "components/modelComponent.hpp"
#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
#include "utils.hpp"

#include <SDL3/SDL.h>

// TODO: PlanetActor
Sun::Sun(class Game* game) : Actor(game) {
	ModelComponent* const sun =
		new ModelComponent(this, getGame()->fullPath("models" SEPARATOR "earth.obj"), false);
	sun->setShaders("sun.vert", "sun.frag");

	setScale(5.0f);

	// TODO: LightComponent
	getGame()->getRenderer()->addPointLight(this);
}

void Sun::updateActor(float delta) {
	(void)delta;

	float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;

	Eigen::Vector3f pos(cos(time) * 12.0f, 5.0f, sin(time) * 12.0f);

	setPosition(pos);
}

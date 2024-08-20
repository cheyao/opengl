#include "actors/view.hpp"

#include "components/cameraComponent.hpp"

View::View(class Game* game) : Actor(game) {
	new CameraComponent(this);
}

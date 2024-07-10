#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "game.hpp"

Player::Player(class Game* owner) : Actor(owner) {
	new CameraComponent(this);
	setPosition(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
}

#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "components/movementComponent.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>

Player::Player(class Game* owner) : Actor(owner), up(0) {
	new CameraComponent(this, true);
	mMoveComp = new MovementComponent(this);

	setPosition(Eigen::Vector3f(0.0f, 6.0f, 10.0f));
}

void Player::actorInput(const bool* keystate) {
}

void Player::updateActor(float delta) {
}

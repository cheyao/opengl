#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "components/movementComponent.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>

Player::Player(class Game* owner) : Actor(owner) {
	new CameraComponent(this);
	mMoveComp = new MovementComponent(this);

	setPosition(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
}

void Player::actorInput(const uint8_t* keystate) {
	if (keystate[SDL_SCANCODE_UP]) {
		mMoveComp->setForwardSpeed(5.0);
	} else if (keystate[SDL_SCANCODE_DOWN]) {
		mMoveComp->setForwardSpeed(-5.0);
	} else {
		mMoveComp->setForwardSpeed(0.0);
	}
}

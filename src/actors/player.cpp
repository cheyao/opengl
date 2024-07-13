#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "components/movementComponent.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>

Player::Player(class Game* owner) : Actor(owner), move(0) {
	new CameraComponent(this);
	mMoveComp = new MovementComponent(this);

	setPosition(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
}

void Player::actorInput(const uint8_t* keystate) {
	if (keystate[SDL_SCANCODE_W]) {
		mMoveComp->setForwardSpeed(5.0);
	} else if (keystate[SDL_SCANCODE_S]) {
		mMoveComp->setForwardSpeed(-5.0);
	} else {
		mMoveComp->setForwardSpeed(0.0);
	}

	if (keystate[SDL_SCANCODE_D]) {
		mMoveComp->setStrafeSpeed(5.0);
	} else if (keystate[SDL_SCANCODE_A]) {
		mMoveComp->setStrafeSpeed(-5.0);
	} else {
		mMoveComp->setStrafeSpeed(0.0);
	}

	SDL_Keymod mod = SDL_GetModState();
	if (mod & SDL_KMOD_SHIFT) {
		move = -5;
	} else if (keystate[SDL_SCANCODE_SPACE]) {
		move = 5;
	} else {
		move = 0;
	}
}

void Player::updateActor(float delta) {
	Eigen::Vector3f pos = getPosition();
	pos.y() += move * delta;
	setPosition(pos);
}

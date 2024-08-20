#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "components/inputComponent.hpp"
#include "components/sprite2DComponent.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>

Player::Player(class Game* game) : Actor(game) {
	new CameraComponent(this, true);

	InputComponent* component = new InputComponent(this);
	component->addCalback(SDL_SCANCODE_UP, [this] { this->setPosition(Eigen::Vector3f(1.0f, 1.0f, 1.0f)); });

	/*Sprite2DComponent* spriteComponent = */ new Sprite2DComponent(this, getGame()->getTexture("stone.png"));
}

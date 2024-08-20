#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "components/inputComponent.hpp"
#include "components/physicsComponent.hpp"
#include "components/sprite2DComponent.hpp"
#include "game.hpp"

Player::Player(class Game* game) : Actor(game) {
	new CameraComponent(this, true);

	PhysicsComponent* physics = new PhysicsComponent(this);

	InputComponent* component = new InputComponent(this);
	component->addCalback(SDL_SCANCODE_UP, [physics] { physics->addVelocity(Eigen::Vector3f(0.0f, 1.0f, 0.0f)); });
	component->addCalback(SDL_SCANCODE_LEFT,
			      [physics] { physics->addVelocity(Eigen::Vector3f(-1.0f, 0.0f, 0.0f)); });
	component->addCalback(SDL_SCANCODE_RIGHT,
			      [physics] { physics->addVelocity(Eigen::Vector3f(1.0f, 0.0f, 0.0f)); });
	component->addCalback(SDL_SCANCODE_DOWN,
			      [physics] { physics->addVelocity(Eigen::Vector3f(0.0f, -1.0f, 0.0f)); });

	Sprite2DComponent* spriteComponent = new Sprite2DComponent(this, getGame()->getTexture("stone.png"));
	spriteComponent->setShaders("block.vert", "block.frag");
}

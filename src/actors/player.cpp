#include "actors/player.hpp"

#include "components/cameraComponent.hpp"
#include "components/collisionComponent.hpp"
#include "components/inputComponent.hpp"
#include "components/physicsComponent.hpp"
#include "components/rectangleCollisionComponent.hpp"
#include "components/sprite2DComponent.hpp"
#include "game.hpp"

#include <SDL3/SDL.h>

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

	RectangleCollisionComponent* collisionComponent =
		new RectangleCollisionComponent(this, spriteComponent->getSize());
	collisionComponent->onCollide([](CollisionComponent* that) { that->getOwner()->setState(Actor::DEAD); });
}

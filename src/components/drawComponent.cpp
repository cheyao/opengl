#include "components/drawComponent.hpp"

#include "actors/actor.hpp"
#include "game.hpp"
#include "opengl/renderer.hpp"

DrawComponent::DrawComponent(class Actor* owner, int drawOrder)
	: Component(owner), mDrawOrder(drawOrder), mVisible(true) {
	SDL_Log("Called");
	mShader = mOwner->getGame()->getShader("basic.vert", "basic.frag");

	mOwner->getGame()->getRenderer()->addSprite(this);
}

void DrawComponent::draw() {}

#include "components/drawComponent.hpp"

#include "actors/actor.hpp"
#include "game.hpp"
#include "opengl/renderer.hpp"

DrawComponent::DrawComponent(class Actor* owner, int drawOrder)
	: Component(owner), mDrawOrder(drawOrder), mVisible(true) {
	mShader = mOwner->getGame()->getShader("basic.vert", "basic.frag");
	mOwner->getGame()->getRenderer()->addSprite(this);
}

DrawComponent::~DrawComponent() {}

void DrawComponent::draw() {}

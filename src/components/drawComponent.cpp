#include "components/drawComponent.hpp"

#include "actors/actor.hpp"
#include "game.hpp"
#include "opengl/renderer.hpp"

DrawComponent::DrawComponent(class Actor* owner, int drawOrder)
	: Component(owner), mDrawOrder(drawOrder), mVisible(true), mVert("default.vert"), mFrag("default.frag"), mShader(nullptr) {
	reload();
	// NOTE: Add sprite of renderer needs shader
	mOwner->getGame()->getRenderer()->addSprite(this);
}

void DrawComponent::reload() {
	mShader = mOwner->getGame()->getShader(mVert, mFrag);
}

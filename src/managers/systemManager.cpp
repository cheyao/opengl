#include "managers/systemManager.hpp"

#include "game.hpp"
#include "systems/physicsSystem.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"

#include <memory>

SystemManager::SystemManager(Game* game) : mGame(game) {
	// mPhysicsSystem = std::make_unique<PhysicsSystem>();
	mRenderSystem = std::make_unique<RenderSystem>(mGame);
	mTextSystem = std::make_unique<TextSystem>(mGame);
}

SystemManager::~SystemManager() {}

void SystemManager::setDemensions(const int width, const int height) { mRenderSystem->setDemensions(width, height); }

void SystemManager::update(const float delta) {
	(void) delta;
	// TODO:
	// mPhysicsSystem->update();
	mRenderSystem->draw();
	mTextSystem->draw();

	mRenderSystem->present();
}

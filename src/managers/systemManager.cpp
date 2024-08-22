#include "managers/systemManager.hpp"

#include "game.hpp"
#include "scene.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"
#include "third_party/Eigen/Core"

#include <memory>

SystemManager::SystemManager(Game* game) : mGame(game) {
	// mPhysicsSystem = std::make_unique<PhysicsSystem>();
	mRenderSystem = std::make_unique<RenderSystem>(mGame);
	mTextSystem = std::make_unique<TextSystem>(mGame);
}

SystemManager::~SystemManager() {}

void SystemManager::setDemensions(const int width, const int height) { mRenderSystem->setDemensions(width, height); }

Eigen::Vector2f SystemManager::getDemensions() const {
	return Eigen::Vector2f(mRenderSystem->getWidth(), mRenderSystem->getHeight());
}

void SystemManager::update(Scene* scene, const float delta) {
	(void)delta;
	// TODO:
	// mPhysicsSystem->update();
	mRenderSystem->draw(scene);
	mTextSystem->draw(scene);

	mRenderSystem->present();
}

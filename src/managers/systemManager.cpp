#include "managers/systemManager.hpp"

#include "game.hpp"
#include "scene.hpp"
#include "systems/inputSystem.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"
#include "third_party/Eigen/Core"

#include <memory>

SystemManager::SystemManager(Game* game) : mGame(game) {
	// mPhysicsSystem = std::make_unique<PhysicsSystem>();
	mRenderSystem = std::make_unique<RenderSystem>(mGame);
	mTextSystem = std::make_unique<TextSystem>(mGame);
	mInputSystem = std::make_unique<InputSystem>(mGame);
}

SystemManager::~SystemManager() {}

void SystemManager::setDemensions(const int width, const int height) { mRenderSystem->setDemensions(width, height); }

Eigen::Vector2f SystemManager::getDemensions() const {
	return Eigen::Vector2f(mRenderSystem->getWidth(), mRenderSystem->getHeight());
}

Texture* SystemManager::getTexture(const std::string& name, const bool srgb) {
	return mRenderSystem->getTexture(name, srgb);
}

void SystemManager::update(Scene* scene, const float delta) {
	mInputSystem->update(scene, delta);
	// mPhysicsSystem->update();
	
	mRenderSystem->draw(scene);
	// mTextSystem->draw(scene);

	mRenderSystem->present();
}

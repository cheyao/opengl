#include "managers/systemManager.hpp"

#include "game.hpp"
#include "scene.hpp"
#include "systems/inputSystem.hpp"
#include "systems/physicsSystem.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"
#include "third_party/Eigen/Core"

#include <memory>

#ifdef IMGUI
#include "imgui.h"
#endif

SystemManager::SystemManager(Game* game) : mGame(game) {
	mPhysicsSystem = std::make_unique<PhysicsSystem>(mGame);
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

Shader* SystemManager::getShader(const std::string& vert, const std::string& frag, const std::string& geom) {
	return mRenderSystem->getShader(vert, frag, geom);
}

void SystemManager::update(Scene* scene, const float delta) {
	assert(scene != nullptr);
	scene->clearSignals();

	mPhysicsSystem->collide(scene);

	mInputSystem->update(scene, delta);
	mPhysicsSystem->update(scene, delta);

	mRenderSystem->draw(scene);
	mTextSystem->draw(scene);

#ifdef IMGUI
	// Print out signals
	static bool signal = false;
	ImGui::Begin("Main menu");
	ImGui::Checkbox("List of signals", &signal);
	ImGui::End();

	if (signal) {
		ImGui::Begin("Signal list");

		for (const auto& [name, value] : scene->mSignals) {
			ImGui::BulletText("%s", std::format("{}: {}", name, value).data());
		}

		ImGui::End();
	}
#endif

	mRenderSystem->present();
}

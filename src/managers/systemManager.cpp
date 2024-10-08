#include "managers/systemManager.hpp"

#include "components.hpp"
#include "scene.hpp"
#include "systems/inputSystem.hpp"
#include "systems/physicsSystem.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <memory>

#ifdef IMGUI
#include "imgui.h"

#include <cstdint>
#include <format>
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
	SDL_assert(scene != nullptr);

	scene->clearSignals();

	mInputSystem->update(scene, delta);
	mPhysicsSystem->update(scene, delta);

	mPhysicsSystem->collide(scene);

	mRenderSystem->draw(scene);
	mTextSystem->draw(scene);

	printDebug(scene);

	mRenderSystem->present();
}

void SystemManager::registerClick(const float x, const float y) {
	mInputSystem->registerClick(x, y);
}

void SystemManager::printDebug([[maybe_unused]] Scene* scene) {
#ifdef IMGUI
	// Print out signals
	static bool signalList = false;
	static bool entityList = false;
	ImGui::Begin("Main menu");
	ImGui::Checkbox("List of signals", &signalList);
	ImGui::Checkbox("List of entities", &entityList);
	ImGui::End();

	if (signalList) {
		ImGui::Begin("Signal list");

		for (const auto& [name, value] : scene->mSignals) {
			ImGui::BulletText("%s", std::format("{}: {}", name, value).data());
		}

		ImGui::End();
	}

	if (entityList) {
		ImGui::Begin("Entity list");

		// TODO: Find some better way then to manually check
		for (const auto& entity : scene->mDebugEntities) {
			if (ImGui::TreeNode(std::format("Entity {}", entity).data())) {
				if (scene->contains<Components::position>(entity)) {
					/*
					ImGui::BulletText(
						"Components::position %s",
						std::format("pos {}",
							    scene->get<Components::position>(entity).mPosition)
							.data());
					*/
				}

				if (scene->contains<Components::velocity>(entity)) {
					ImGui::BulletText(
						"Components::velocity %s",
						std::format("vel {}",
							    scene->get<Components::velocity>(entity).mVelocity)
							.data());
				}

				if (scene->contains<Components::collision>(entity)) {
					ImGui::BulletText(
						"Components::collision %s",
						std::format("offset {} size {} stationary {}",
							    scene->get<Components::collision>(entity).mOffset,
							    scene->get<Components::collision>(entity).mSize,
							    scene->get<Components::collision>(entity).mStationary)
							.data());
				}

				if (scene->contains<Components::texture>(entity)) {
					// TODO: Hight width
					ImGui::BulletText(
						"Components::texture %s",
						std::format("texture {} shader {} scale {}",
							    reinterpret_cast<std::uintptr_t>(
								    scene->get<Components::texture>(entity).mTexture),
							    reinterpret_cast<std::uintptr_t>(
								    scene->get<Components::texture>(entity).mShader),
							    scene->get<Components::texture>(entity).mScale)
							.data());
				}

				if (scene->contains<Components::input>(entity)) {
					ImGui::BulletText(
						"Components::input %s",
						std::format("{}",
							    scene->get<Components::input>(entity).mFunction != nullptr)
							.data());
				}

				if (scene->contains<Components::text>(entity)) {
					ImGui::BulletText(
						"Components::text %s",
						std::format("id {}", scene->get<Components::text>(entity).mID).data());
				}

				if (scene->contains<Components::misc>(entity)) {
					ImGui::BulletText(
						"Components::misc %s",
						std::format("what {}", scene->get<Components::misc>(entity).mWhat)
							.data());
				}

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}
#endif
}

#include "managers/systemManager.hpp"

#include "components.hpp"
#include "components/playerInventory.hpp"
#include "game.hpp"
#include "managers/eventManager.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"
#include "systems/inputSystem.hpp"
#include "systems/physicsSystem.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <memory>

#ifdef IMGUI
#include "imgui.h"

#include <concepts>
#include <format>
#include <sstream>
#include <string_view>
#include <type_traits>

template <typename EigenExprTypeT>
concept EigenTypeMatExpr = requires(const EigenExprTypeT t) {
	std::remove_cvref_t<EigenExprTypeT>::RowsAtCompileTime;
	std::remove_cvref_t<EigenExprTypeT>::ColsAtCompileTime;
	typename std::remove_cvref_t<EigenExprTypeT>::Scalar;
	{ t.size() } -> std::same_as<typename Eigen::Index>;
	{ t.rows() } -> std::same_as<typename Eigen::Index>;
	{ t.cols() } -> std::same_as<typename Eigen::Index>;
};
enum class EigenCustomFormats {
	Default,	     //
	CleanFormat,	     // cf
	HeavyFormat,	     // hf
	SingleLineFormat,    // sfl
	HighPrecisionFormat, // hpf
	DebuggingFormat	     // df
};
static const auto defaultFormat = Eigen::IOFormat();
static const auto cleanFormat = Eigen::IOFormat(4, 0, ", ", "\n", "[", "]");
static const auto heavyFormat = Eigen::IOFormat(Eigen::FullPrecision, 0, ", ", ";\n", "[", "]", "[", "]");
static const auto singleLineFormat =
	Eigen::IOFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "", "", "", "");
static const auto highPrecisionFormat =
	Eigen::IOFormat(Eigen::FullPrecision, Eigen::DontAlignCols, " ", "\n", "", "", "", "");
static const auto debuggingFormat =
	Eigen::IOFormat(Eigen::FullPrecision, Eigen::DontAlignCols, " ", "\n", "", "", "\n", "");
template <EigenTypeMatExpr MatT> struct std::formatter<MatT> {
	constexpr auto parse(format_parse_context& ctx) {
		const std::string_view fmt(ctx.begin(), ctx.end());
		if (fmt.starts_with("cf"))
			_format = EigenCustomFormats::CleanFormat;
		if (fmt.starts_with("hf"))
			_format = EigenCustomFormats::HeavyFormat;
		if (fmt.starts_with("sfl"))
			_format = EigenCustomFormats::SingleLineFormat;
		if (fmt.starts_with("hpf"))
			_format = EigenCustomFormats::HighPrecisionFormat;
		if (fmt.starts_with("df"))
			_format = EigenCustomFormats::DebuggingFormat;
		return ctx.begin() + fmt.find_first_of('}');
	}
	template <typename FormatContext> auto format(const MatT& m, FormatContext& ctx) const {
		switch (_format) {
			case EigenCustomFormats::CleanFormat:
				return std::format_to(
					ctx.out(), "{}",
					(std::stringstream{} << std::fixed << m.format(cleanFormat)).str());
			case EigenCustomFormats::HeavyFormat:
				return std::format_to(
					ctx.out(), "{}",
					(std::stringstream{} << std::fixed << m.format(heavyFormat)).str());
			case EigenCustomFormats::SingleLineFormat:
				return std::format_to(
					ctx.out(), "{}",
					(std::stringstream{} << std::fixed << m.format(singleLineFormat)).str());
			case EigenCustomFormats::HighPrecisionFormat:
				return std::format_to(
					ctx.out(), "{}",
					(std::stringstream{} << std::fixed << m.format(highPrecisionFormat)).str());
			case EigenCustomFormats::DebuggingFormat:
				return std::format_to(
					ctx.out(), "{}",
					(std::stringstream{} << std::fixed << m.format(debuggingFormat)).str());
			default:
				return std::format_to(
					ctx.out(), "{}",
					(std::stringstream{} << std::fixed << m.format(defaultFormat)).str());
		}
	}

      private:
	EigenCustomFormats _format{EigenCustomFormats::Default};
};
template <EigenTypeMatExpr MatT> std::ostream& operator<<(std::ostream& os, const MatT& mat) {
	return os << std::format("{:hf}", mat);
}
#endif

SystemManager::SystemManager() noexcept
	: mPhysicsSystem(std::make_unique<PhysicsSystem>()), mRenderSystem(std::make_unique<RenderSystem>()),
	  mInputSystem(std::make_unique<InputSystem>()), mTextSystem(std::make_unique<TextSystem>()),
	  mUISystem(std::make_unique<UISystem>()) {}

SystemManager::~SystemManager() { SDL_Log("Unloading system"); }

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

// 83.3% of the time
void SystemManager::update(Scene* scene, const float delta) {
	SDL_assert(scene != nullptr);

	mUISystem->update(scene, delta);
	mPhysicsSystem->update(scene, delta); // 12.08%

	mPhysicsSystem->collide(scene); // 33.72%

	updatePlayer(scene);

	// This is after since it will delete stuff
	mInputSystem->update(scene, delta);

	mRenderSystem->draw(scene); // 36.51%
	mInputSystem->draw(scene);
	mTextSystem->draw(scene);
	mUISystem->draw(scene);

	printDebug(scene);

	mRenderSystem->present();

	scene->getSignal(EventManager::LEFT_CLICK_DOWN_SIGNAL) = false;
	scene->getSignal(EventManager::RIGHT_CLICK_DOWN_SIGNAL) = false;
}

void SystemManager::updatePlayer(Scene* scene) {
	// We handle some player's logic here
	if (!mUISystem->empty()) {
		return;
	}

	const auto select = [&scene](SDL_Scancode s, std::size_t n) {
		if (scene->getSignal(s)) {
			static_cast<PlayerInventory*>(
				scene->get<Components::inventory>(Game::getInstance()->getPlayerID()).mInventory)
				->select(n);

			scene->getSignal(s) = false;
		}
	};

	select(SDL_SCANCODE_1, 0);
	select(SDL_SCANCODE_2, 1);
	select(SDL_SCANCODE_3, 2);
	select(SDL_SCANCODE_4, 3);
	select(SDL_SCANCODE_5, 4);
	select(SDL_SCANCODE_6, 5);
	select(SDL_SCANCODE_7, 6);
	select(SDL_SCANCODE_8, 7);
	select(SDL_SCANCODE_9, 8);
}

void SystemManager::printDebug([[maybe_unused]] Scene* scene) {
#ifdef IMGUI
	// Print out signals
	static bool signalList = false;
	static bool entityList = false;
	ImGui::Begin("Developer menu");
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

	/*
	if (entityList) {
		ImGui::Begin("Entity list");

		// TODO: Find some better way then to manually check
		for (const auto& entity : scene->mDebugEntities) {
			if (ImGui::TreeNode(std::format("Entity {}", entity).data())) {
				if (scene->contains<Components::position>(entity)) {
					ImGui::BulletText(
						"Components::position %s",
						std::format("pos {}",
							    scene->get<Components::position>(entity).mPosition)
							.data());
				}

				if (scene->contains<Components::block>(entity)) {
					ImGui::BulletText(
						"Components::block %s",
						std::format("pos {}", scene->get<Components::block>(entity).mPosition)
							.data());
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
	*/
#endif
}

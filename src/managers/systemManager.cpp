#include "managers/systemManager.hpp"

#include "components.hpp"
#include "scene.hpp"
#include "systems/UISystem.hpp"
#include "systems/inputSystem.hpp"
#include "systems/physicsSystem.hpp"
#include "systems/renderSystem.hpp"
#include "systems/textSystem.hpp"
#include "third_party/Eigen/Core"

#include <SDL3/SDL.h>
#include <functional>
#include <memory>

#ifdef IMGUI
#include "imgui.h"

#include <concepts>
#include <cstdint>
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

SystemManager::SystemManager(Game* game)
	: mGame(game), mPhysicsSystem(std::make_unique<PhysicsSystem>(mGame)),
	  mRenderSystem(std::make_unique<RenderSystem>(mGame)), mTextSystem(std::make_unique<TextSystem>(mGame)),
	  mInputSystem(std::make_unique<InputSystem>(mGame)), mUISystem(new UISystem(mGame)) {}

SystemManager::~SystemManager() { delete mUISystem; }

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

	scene->clearSignals();

	mUISystem->update(scene,delta);
	mInputSystem->update(scene, delta);
	mPhysicsSystem->update(scene, delta); // 12.08%

	mPhysicsSystem->collide(scene); // 33.72%

	mRenderSystem->draw(scene); // 36.51%
	mInputSystem->draw(scene);
	mTextSystem->draw(scene);
	mUISystem->draw(scene);

	printDebug(scene);

	mRenderSystem->present();
}

void SystemManager::registerClick(const float x, const float y) { mInputSystem->registerClick(x, y); }

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

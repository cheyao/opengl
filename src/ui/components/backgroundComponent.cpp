#include "ui/components/backgroundComponent.hpp"

#include "third_party/Eigen/Core"
#include "third_party/glad/glad.h"
#include "ui/UIComponent.hpp"
#include "ui/UIScreen.hpp"
#include <format>

BackgroundComponent::BackgroundComponent(class UIScreen* owner, const Eigen::Vector3f color)
	: UIComponent(owner, Eigen::Vector2f::Zero(), "BackgroundComponent"), mColor(color) {}

// PERF: Stop cleaning when multiple backgrounds
void BackgroundComponent::draw([[maybe_unused]] const class Shader* shader) {
	glClearColor(mColor.x(), mColor.y(), mColor.z(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

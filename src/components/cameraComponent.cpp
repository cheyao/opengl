#include "components/cameraComponent.hpp"

#include "actors/actor.hpp"
#include "components/component.hpp"
#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Geometry"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cmath>

CameraComponent::CameraComponent(Actor* owner, int priority)
	: Component(owner, priority), mFOV(45), mViewMatrix(Eigen::Affine3f::Identity()),
	  mProjectionMatrix(Eigen::Affine3f::Identity()) {
	Eigen::Quaternionf dir = mOwner->getRotation();
	Eigen::AngleAxisf rot(toRadians(90), Eigen::Vector3f::UnitY());
	mOwner->setRotation(dir * rot);

	project(); // NOTE: Order important, the game will set the matrix when we set the camera

	mOwner->getGame()->getRenderer()->setCamera(this);
};

void CameraComponent::update([[maybe_unused]] float delta) {
	float x = 0;
	float y = 0;
	SDL_GetRelativeMouseState(&x, &y);

	const Eigen::Quaternionf dir = mOwner->getRotation();
	const Eigen::AngleAxisf rot(-(x / 50) * delta, Eigen::Vector3f::UnitY());
	const Eigen::AngleAxisf yaw(-(y / 50) * delta, Eigen::Vector3f::UnitZ());
	mOwner->setRotation(dir * rot * yaw);

	view();
}

void CameraComponent::view() {
	Eigen::Matrix3f R;
	const Eigen::Vector3f up(0.0f, 1.0f, 0.0f);
	R.col(2) = (-mOwner->getForward() * 100.f).normalized();
	R.col(0) = up.cross(R.col(2)).normalized();
	R.col(1) = R.col(2).cross(R.col(0));
	mViewMatrix.matrix().topLeftCorner<3, 3>() = R.transpose();
	mViewMatrix.matrix().topRightCorner<3, 1>() = -R.transpose() * mOwner->getPosition();
	mViewMatrix(3, 3) = 1.0f;
}

void CameraComponent::project() {
	constexpr const float near = 0.1f;
	constexpr const float far = 100.0f;
	const float aspect = static_cast<float>(mOwner->getGame()->getWidth()) / mOwner->getGame()->getHeight();
	const float theta = mFOV * 0.5;
	constexpr const float range = far - near;
	const float invtan = 1.0f / tan(theta);

	// https://www.songho.ca/opengl/gl_projectionmatrix.html
	mProjectionMatrix(0, 0) = invtan / aspect;
	mProjectionMatrix(1, 1) = invtan;
	mProjectionMatrix(2, 2) = -(near + far) / range;
	mProjectionMatrix(3, 2) = -1;
	mProjectionMatrix(2, 3) = -2 * near * far / range;
	mProjectionMatrix(3, 3) = 0;
}

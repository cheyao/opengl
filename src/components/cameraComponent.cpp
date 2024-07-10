#include "components/cameraComponent.hpp"

#include "components/component.hpp"
#include "game.hpp"
#include "third_party/Eigen/Dense"

#include <cmath>

CameraComponent::CameraComponent(Actor* owner, int priority) : Component(owner, priority), mFOV(45) {
	mProjectionMatrix = Eigen::Affine3f::Identity();
};

CameraComponent::~CameraComponent() {}

void CameraComponent::update(float delta) {
	project();
	view(Eigen::Vector3f(0.0f, 0.0f, 0.0f), Eigen::Vector3f(0.0f, 1.0f, 0.0f));

	mOwner->getGame()->mView = mViewMatrix;
	mOwner->getGame()->mProjection = mProjectionMatrix;

	(void)delta;
}

void CameraComponent::view(const Eigen::Vector3f& target, const Eigen::Vector3f& up) {
	Eigen::Matrix3f R;
	R.col(2) = (mOwner->getPosition() - target).normalized();
	R.col(0) = up.cross(R.col(2)).normalized();
	R.col(1) = R.col(2).cross(R.col(0));
	mViewMatrix.matrix().topLeftCorner<3, 3>() = R.transpose();
	mViewMatrix.matrix().topRightCorner<3, 1>() = -R.transpose() * mOwner->getPosition();
	mViewMatrix(3, 3) = 1.0f;
}

void CameraComponent::project() {
	const float near = 0.1f;
	const float far = 100.0f;
	const float aspect =
		static_cast<float>(mOwner->getGame()->getWidth()) / mOwner->getGame()->getHeight();
	float theta = mFOV * 0.5;
	float range = far - near;
	float invtan = 1. / tan(theta);

	// https://www.songho.ca/opengl/gl_projectionmatrix.html
	mProjectionMatrix(0, 0) = invtan / aspect;
	mProjectionMatrix(1, 1) = invtan;
	mProjectionMatrix(2, 2) = -(near + far) / range;
	mProjectionMatrix(3, 2) = -1;
	mProjectionMatrix(2, 3) = -2 * near * far / range;
	mProjectionMatrix(3, 3) = 0;
}

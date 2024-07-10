#include "opengl/camera.hpp"
#include "third_party/Eigen/src/Core/Matrix.h"

#include <third_party/Eigen/Dense>

#include <cmath>

Camera::Camera() { mProjectionMatrix = Eigen::Affine3f::Identity(); };

Camera::~Camera() {}

void Camera::view(const Eigen::Vector3f& position, const Eigen::Vector3f& target,
				  const Eigen::Vector3f& up) {
	Eigen::Matrix3f R;
	R.col(2) = (position - target).normalized();
	R.col(0) = up.cross(R.col(2)).normalized();
	R.col(1) = R.col(2).cross(R.col(0));
	mViewMatrix.matrix().topLeftCorner<3, 3>() = R.transpose();
	mViewMatrix.matrix().topRightCorner<3, 1>() = -R.transpose() * position;
	mViewMatrix(3, 3) = 1.0f;
}

void Camera::project(const float& fov, const int& w, const int& h, const float& near,
					 const float& far) {
	const float aspect = static_cast<float>(w) / h;
	float theta = fov * 0.5;
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

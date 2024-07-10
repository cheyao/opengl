#pragma once

#include <third_party/Eigen/Dense>

class Camera {
  public:
	Camera();
	Camera(Camera&&) = delete;
	Camera(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	~Camera();

	void project(const float& fov, const int& w, const int& h, const float& near, const float& far);
	void view(const Eigen::Vector3f& position, const Eigen::Vector3f& target,
			  const Eigen::Vector3f& up);

	Eigen::Affine3f mViewMatrix;
	Eigen::Affine3f mProjectionMatrix;
};

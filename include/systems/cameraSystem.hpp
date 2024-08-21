#pragma once

#include "systems/system.hpp"
#include "third_party/Eigen/Geometry"

class CameraSystem : public System {
      public:
	explicit CameraSystem(class Actor* owner, bool ortho = false, int priority = 200);
	CameraSystem(CameraSystem&&) = delete;
	CameraSystem(const CameraSystem&) = delete;
	CameraSystem& operator=(CameraSystem&&) = delete;
	CameraSystem& operator=(const CameraSystem&) = delete;
	~CameraSystem() override = default;

	void update(float delta) override;

	void setFOV(float fov) { mFOV = fov; }

	Eigen::Affine3f getViewMatrix() {
		view();
		return mViewMatrix;
	};
	Eigen::Affine3f getProjectionMatrix() {
		project();
		return mProjectionMatrix;
	};

      private:
	void project();
	void view();

	void ortho();
	void persp();

	bool mOrtho;

	float mFOV;

	Eigen::Affine3f mViewMatrix;
	Eigen::Affine3f mProjectionMatrix;
};

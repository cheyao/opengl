#pragma once

#include "components/component.hpp"
#include "third_party/Eigen/Geometry"

class CameraSystem : public Component {
      public:
	explicit CameraSystem(class Actor* owner, bool ortho = false, int priority = 200);
	CameraSystem(CameraSystem&&) = delete;
	CameraSystem(const CameraSystem&) = delete;
	CameraSystem& operator=(CameraSystem&&) = delete;
	CameraSystem& operator=(const CameraSystem&) = delete;
	~CameraSystem() override = default;

	void update(float delta) override;

	void setFOV(float fov) { mFOV = fov; }

	class Actor* getOwner() { return mOwner; }

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

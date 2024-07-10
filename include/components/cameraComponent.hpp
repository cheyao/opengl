#pragma once

#include "actors/actor.hpp"
#include "components/component.hpp"
#include "third_party/Eigen/Dense"

class CameraComponent : public Component {
  public:
	explicit CameraComponent(class Actor* owner, int priority = 200);
	CameraComponent(CameraComponent&&) = delete;
	CameraComponent(const CameraComponent&) = delete;
	CameraComponent& operator=(CameraComponent&&) = delete;
	CameraComponent& operator=(const CameraComponent&) = delete;
	~CameraComponent();

	void update(float delta) override;

	void setFOV(float fov) { mFOV = fov; }

	void project();
	void view(const Eigen::Vector3f& target, const Eigen::Vector3f& up);

	Eigen::Affine3f mViewMatrix;
	Eigen::Affine3f mProjectionMatrix;

	float mFOV;
};

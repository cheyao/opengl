#pragma once

#include "components/drawComponent.hpp"
#include "third_party/Eigen/Core"
#include "actors/actor.hpp"

class Sprite2DComponent : public DrawComponent {
      public:
	explicit Sprite2DComponent(class Actor* owner, class Texture* texture, int drawOrder = 100);
	Sprite2DComponent(Sprite2DComponent&&) = delete;
	Sprite2DComponent(const Sprite2DComponent&) = delete;
	Sprite2DComponent& operator=(Sprite2DComponent&&) = delete;
	Sprite2DComponent& operator=(const Sprite2DComponent&) = delete;
	~Sprite2DComponent() override = default;

	void draw() override;
	Eigen::Vector2f getSize() const { return mSize * mOwner->getScale(); }

      private:
	const Eigen::Vector2f mSize;
	class Mesh* mMesh;
};

#pragma once

#include "components/drawComponent.hpp"

class Sprite2DComponent : public DrawComponent {
      public:
	explicit Sprite2DComponent(class Actor* owner, class Texture* texture, int drawOrder = 100);
	Sprite2DComponent(Sprite2DComponent&&) = delete;
	Sprite2DComponent(const Sprite2DComponent&) = delete;
	Sprite2DComponent& operator=(Sprite2DComponent&&) = delete;
	Sprite2DComponent& operator=(const Sprite2DComponent&) = delete;
	~Sprite2DComponent() override = default;

	void draw() override;

      private:
	class Mesh* mMesh;
};

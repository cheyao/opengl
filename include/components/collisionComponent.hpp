#pragma once

#include "components/component.hpp"
#include <functional>

class CollisionComponent : public Component {
      public:
	explicit CollisionComponent(class Actor* owner, int updatePriority = 100);
	CollisionComponent(CollisionComponent&&) = delete;
	CollisionComponent(const CollisionComponent&) = delete;
	CollisionComponent& operator=(CollisionComponent&&) = delete;
	CollisionComponent& operator=(const CollisionComponent&) = delete;
	~CollisionComponent() override;

	virtual void collide(class CollisionComponent* that) {
		if (mOnCollide != nullptr) {
			mOnCollide(that);
		}
	}

	void onCollide(std::function<void(class CollisionComponent* that)> func) { mOnCollide = func; }

	int getCollisionPriority() { return mCollisionPriority; }

      private:
	const int mCollisionPriority;

	std::function<void(class CollisionComponent* that)> mOnCollide;
};

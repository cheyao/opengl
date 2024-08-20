#pragma once

#include "components/component.hpp"

class CollisionComponent : public Component {
      public:
	explicit CollisionComponent(class Actor* owner, int updatePriority = 100);
	CollisionComponent(CollisionComponent&&) = delete;
	CollisionComponent(const CollisionComponent&) = delete;
	CollisionComponent& operator=(CollisionComponent&&) = delete;
	CollisionComponent& operator=(const CollisionComponent&) = delete;
	~CollisionComponent() override = default;

	virtual void onColide(class CollisionComponent* that) = 0;

	int getCollisionPriority() { return mCollisionPriority; }

      private:
	const int mCollisionPriority;
};

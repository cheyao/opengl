#pragma once

#include <cstdint>

class Component {
  public:
	explicit Component(class Actor* owner, int priority = 100);
	Component(Component&&) = default;
	Component(const Component&) = default;
	Component& operator=(Component&&) = default;
	Component& operator=(const Component&) = default;
	virtual ~Component();

	virtual void update(float delta);
	virtual void input(const uint8_t* keystate);

	int getUpdatePriority() const { return mUpdatePriority; }

  protected:
	class Actor* mOwner;

	int mUpdatePriority;
};

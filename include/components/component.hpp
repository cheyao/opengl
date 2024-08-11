#pragma once

class Component {
  public:
	explicit Component(class Actor* owner, int priority = 100);
	Component(Component&&) = default;
	Component(const Component&) = default;
	Component& operator=(Component&&) = default;
	Component& operator=(const Component&) = default;
	virtual ~Component();

	virtual void update([[maybe_unused]] float delta);
	virtual void input([[maybe_unused]] const bool* keystate);

	int getUpdatePriority() const { return mUpdatePriority; }

  protected:
	class Actor* mOwner;

	int mUpdatePriority;
};

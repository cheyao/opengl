#pragma once

class Component {
  public:
	explicit Component(class Actor* owner, int priority = 100);
	Component(Component&&) = delete;
	Component(const Component&) = delete;
	Component& operator=(Component&&) = delete;
	Component& operator=(const Component&) = delete;
	virtual ~Component();

	virtual void update([[maybe_unused]] float delta);
	virtual void input([[maybe_unused]] const bool* keystate);

	int getUpdatePriority() const { return mUpdatePriority; }

  protected:
	class Actor* mOwner;

	int mUpdatePriority;
};

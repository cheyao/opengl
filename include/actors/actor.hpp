#pragma once

#include "third_party/Eigen/Geometry"

#include <vector>

class Actor {
  public:
	enum State { ALIVE, PAUSED, DEAD };

	explicit Actor(class Game* game);
	Actor(Actor&&) = delete;
	Actor(const Actor&) = delete;
	Actor& operator=(Actor&&) = delete;
	Actor& operator=(const Actor&) = delete;
	virtual ~Actor();

	void update(float delta);
	void updateComponents(float delta);
	virtual void updateActor([[maybe_unused]] float delta);

	void input(const bool* keystate);
	virtual void actorInput([[maybe_unused]] const bool* keystate);

	[[nodiscard]] class Game* getGame() const { return mGame; }

	[[nodiscard]] State getState() const { return mState; }
	void setState(const State& state) { mState = state; }

	[[nodiscard]] const Eigen::Vector3f& getPosition() const { return mPosition; }
	void setPosition(const Eigen::Vector3f& pos) { mPosition = pos; }

	[[nodiscard]] float getScale() const { return mScale; }
	void setScale(float scale) { mScale = scale; }

	[[nodiscard]] const Eigen::Quaternionf& getRotation() const { return mRotation; }
	void setRotation(const Eigen::Quaternionf& rotation) { mRotation = rotation; }

	[[nodiscard]] Eigen::Vector3f getForward() const {
		return mRotation.toRotationMatrix() * Eigen::Vector3f::UnitX();
	}
	[[nodiscard]] Eigen::Vector3f getRight() const {
		return mRotation.toRotationMatrix() * Eigen::Vector3f::UnitZ();
	}


	void addComponent(class Component* component);
	void removeComponent(class Component* component);

  private:
	State mState;

	Eigen::Vector3f mPosition;
	Eigen::Quaternionf mRotation;
	// TODO: Fix rotation
	float mScale;

	class Game* mGame;
	std::vector<class Component*> mComponents;
};

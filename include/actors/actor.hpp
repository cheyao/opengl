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
	virtual void updateActor(float delta);

	void input(const uint8_t* keystate);
	virtual void actorInput(const uint8_t* keystate);

	class Game* getGame() { return mGame; }

	State getState() const { return mState; }
	void setState(const State& state) { mState = state; }

	const Eigen::Vector3f& getPosition() const { return mPosition; }
	void setPosition(const Eigen::Vector3f& pos) { mPosition = pos; }

	float getScale() const { return mScale; }
	void setScale(float scale) { mScale = scale; }

	const Eigen::Quaternionf& getRotation() const { return mRotation; }
	void setRotation(const Eigen::Quaternionf& rotation) { mRotation = rotation; }

	Eigen::Vector3f getForward() const {
		return mRotation.toRotationMatrix() * Eigen::Vector3f::UnitX();
	}
	Eigen::Vector3f getRight() const {
		return mRotation.toRotationMatrix() * Eigen::Vector3f::UnitY();
	}


	void addComponent(class Component* component);
	void removeComponent(class Component* component);

  private:
	State mState;

	Eigen::Vector3f mPosition;
	Eigen::Quaternionf mRotation;
	float mScale;

	class Game* mGame;
	std::vector<class Component*> mComponents;
};

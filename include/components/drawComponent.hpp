#pragma once

#include "components/component.hpp"

class DrawComponent : public Component {
  public:
	DrawComponent(class Actor* owner, int drawOrder = 100);
	DrawComponent(DrawComponent&&) = delete;
	DrawComponent(const DrawComponent&) = delete;
	DrawComponent& operator=(DrawComponent&&) = delete;
	DrawComponent& operator=(const DrawComponent&) = delete;
	~DrawComponent();

	virtual void draw();

	int getDrawOrder() const { return mDrawOrder; }

	void setVisible(bool visible) { mVisible = visible; }
	bool getVisible() const { return mVisible; }

	void setShader(class Shader* shader) { mShader = shader; }
	class Shader* getShader() const { return mShader; }

  protected:
	int mDrawOrder;
	bool mVisible;
	class Shader* mShader;
};

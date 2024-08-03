#pragma once

#include "components/component.hpp"

#include <string>

class DrawComponent : public Component {
  public:
	DrawComponent(class Actor* owner, int drawOrder = 100);
	DrawComponent(DrawComponent&&) = delete;
	DrawComponent(const DrawComponent&) = delete;
	DrawComponent& operator=(DrawComponent&&) = delete;
	DrawComponent& operator=(const DrawComponent&) = delete;
	~DrawComponent() override = default;

	virtual void draw() = 0;

	int getDrawOrder() const { return mDrawOrder; }

	void setVisible(bool visible) { mVisible = visible; }
	bool getVisible() const { return mVisible; }

	void setVert(const std::string& vert) { mVert = vert; reload(); }
	void setFrag(const std::string& frag) { mFrag = frag; reload(); }
	void setGeom(const std::string& geom) { mGeom = geom; reload(); }
	class Shader* getShader() const { return mShader; };

	void reload();

  protected:
	int mDrawOrder;
	bool mVisible;

	std::string mVert;
	std::string mFrag;
	std::string mGeom;

	class Shader* mShader;
};

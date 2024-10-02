#pragma once

#include "opengl/texture.hpp"
#include "third_party/glad/glad.h"

#include <string>
#include <string_view>

class Cubemap : public Texture {
      public:
	explicit Cubemap(std::string_view path);
	Cubemap(Cubemap&&) = delete;
	Cubemap(const Cubemap&) = delete;
	Cubemap& operator=(Cubemap&&) = delete;
	Cubemap& operator=(const Cubemap&) = delete;
	~Cubemap() = default;

	void activate(unsigned int num) const override;
	void load(bool srgb) override;

      private:
	void loadface(const std::string& face, unsigned int i, bool srgb);
};

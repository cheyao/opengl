#pragma once

#include "opengl/texture.hpp"
#include "third_party/glad/glad.h"

#include <string>
#include <string_view>

class Cubemap : public Texture {
      public:
	explicit Cubemap(const std::string_view& path);
	Cubemap(Cubemap&&) = delete;
	Cubemap(const Cubemap&) = delete;
	Cubemap& operator=(Cubemap&&) = delete;
	Cubemap& operator=(const Cubemap&) = delete;
	~Cubemap() = default;

	void activate(const unsigned int& num) const override;
	void load(const bool srgb) override;

      private:
	void loadface(const std::string& face, const unsigned int i, const bool srgb);
};

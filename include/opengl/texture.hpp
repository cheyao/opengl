#pragma once

#include <string>

class Texture {
  public:
	explicit Texture(const std::string_view& path);
	Texture(Texture&&) = delete;
	Texture(const Texture&) = delete;
	Texture& operator=(Texture&&) = delete;
	Texture& operator=(const Texture&) = delete;
	~Texture();

	void activate(const unsigned int& num) const;

  private:
	unsigned int mID;
	std::string name;
};

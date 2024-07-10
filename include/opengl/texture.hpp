#pragma once

#include <string>

class Texture {
  public:
	explicit Texture(const std::string& path, const bool& flip = false);
	Texture(Texture&&) = delete;
	Texture(const Texture&) = delete;
	Texture& operator=(Texture&&) = delete;
	Texture& operator=(const Texture&) = delete;
	~Texture();

	void activate(const unsigned int& num = 0) const;

  private:
	unsigned int mID;
	std::string name;
};

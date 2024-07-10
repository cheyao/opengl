#pragma once

class Actor {
public:
	Actor();
	Actor(Actor &&) = delete;
	Actor(const Actor &) = delete;
	Actor &operator=(Actor &&) = delete;
	Actor &operator=(const Actor &) = delete;
	~Actor();

private:
	
};

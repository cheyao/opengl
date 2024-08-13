#include "utils.hpp"

#include <functional>

int launchThreadHelper(void* fn) {
	std::function<void()> fun = *static_cast<std::function<void()>*>(fn);

	fun();

	return 0;
}


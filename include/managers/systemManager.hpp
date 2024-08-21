#pragma once

#include <vector>
class SystemManager {
      public:
	SystemManager();
	SystemManager(SystemManager&&) = delete;
	SystemManager(const SystemManager&) = delete;
	SystemManager& operator=(SystemManager&&) = delete;
	SystemManager& operator=(const SystemManager&) = delete;
	~SystemManager();

	void update();

      private:
	std::vector<class System*>
};

#include <cstdint>
#include <iostream>

using ComponentID = std::uint64_t;

class ComponentManager {
      public:
	ComponentManager();
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;
	ComponentManager& operator=(const ComponentManager&) = delete;
	~ComponentManager() = default;

	template <typename T> ComponentID getID();

      private:
	int mComponentCount;
};

ComponentManager::ComponentManager() : mComponentCount(0) {}

template <typename T> ComponentID ComponentManager::getID() {
	static int componentID = mComponentCount++;
	return componentID;
}
struct a {
	int b;
};

struct b {
	int b;
};

struct c {
	int b;
};

int main() {
	ComponentManager* cm = new ComponentManager;
	std::cout << cm->getID<a>() << std::endl;
	std::cout << cm->getID<c>() << std::endl;
	std::cout << cm->getID<b>() << std::endl;
	std::cout << cm->getID<a>() << std::endl;
	std::cout << cm->getID<a>() << std::endl;
	std::cout << cm->getID<c>() << std::endl;
}

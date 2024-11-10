#pragma once

#include "managers/entityManager.hpp"
#include <cstddef>
#include <vector>

class Inventory {
      public:
	explicit Inventory(std::size_t size);
	Inventory(Inventory&&) = default;
	Inventory(const Inventory&) = default;
	Inventory& operator=(Inventory&&) = default;
	Inventory& operator=(const Inventory&) = default;
	~Inventory() = default;

      private:
	std::size_t mSize;
	std::vector<EntityID> mVector;
};

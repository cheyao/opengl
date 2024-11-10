#include "components/inventory.hpp"

#include <cstddef>

Inventory::Inventory(const std::size_t size) : mSize(size), mVector(size) {}


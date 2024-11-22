#pragma once

#include "components.hpp"
#include "managers/entityManager.hpp"
#include "screens/screen.hpp"
#include "third_party/json.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

class Inventory : public Screen {
      public:
	explicit Inventory(class Game* game, std::size_t size, EntityID entity = 0);
	explicit Inventory(class Game* game, const nlohmann::json& contents, EntityID entity = 0);
	Inventory(Inventory&&) = delete;
	Inventory(const Inventory&) = delete;
	Inventory& operator=(Inventory&&) = delete;
	Inventory& operator=(const Inventory&) = delete;
	~Inventory() = default;

	void load(const nlohmann::json& contents);
	nlohmann::json save();

	void update(class Scene* scene, float delta) override;
	void draw(class Scene* scene) override;
	// Sees if the entity can pick the item up, true if picked false otherwise
	virtual bool tryPick(Scene* scene, EntityID item);

      private:
	void close();
	void pickUp(Scene* scene, EntityID item, std::size_t index);

	constexpr const static inline char* const SIZE_KEY = "size";
	constexpr const static inline char* const ITEMS_KEY = "items";
	constexpr const static inline char* const COUNT_KEY = "count";

	const EntityID mEntity;
	std::size_t mSize;
	std::vector<Components::Item> mItems;
	std::vector<std::uint64_t> mCount;
};

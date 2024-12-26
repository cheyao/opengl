#pragma once

#include "components.hpp"
#include "managers/entityManager.hpp"
#include "screens/screen.hpp"
#include "systems/renderSystem.hpp"
#include "third_party/json.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

class Inventory : public Screen {
	friend class RenderSystem;

      public:
	explicit Inventory(class Game* game, std::size_t size, EntityID entity = 0);
	explicit Inventory(class Game* game, const nlohmann::json& contents, EntityID entity = 0);
	Inventory(Inventory&&) = delete;
	Inventory(const Inventory&) = delete;
	Inventory& operator=(Inventory&&) = delete;
	Inventory& operator=(const Inventory&) = delete;
	~Inventory() override = default;

	void load(const nlohmann::json& contents);
	nlohmann::json save();

	virtual bool update(class Scene* scene, float delta) override;
	virtual void draw(class Scene* scene) override;
	// Sees if the entity can pick the item up, true if picked up false otherwise
	virtual bool tryPick(class Scene* scene, EntityID item);

      protected:
	const EntityID mEntity;
	std::size_t mSize;
	std::vector<Components::Item> mItems;
	std::vector<std::uint64_t> mCount;

	constexpr const static inline auto INVENTORY_TEXTURE_WIDTH = 176.0f;
	constexpr const static inline auto INVENTORY_TEXTURE_HEIGHT = 166.0f;
	constexpr const static inline auto INVENTORY_SLOTS_OFFSET_X = 7.5f;
	constexpr const static inline auto INVENTORY_SLOTS_OFFSET_Y = 8.5f;
	constexpr const static inline auto INVENTORY_INV_SCALE = 15.0f;
	constexpr const static inline auto INVENTORY_SLOT_X = 18.0f;
	constexpr const static inline auto INVENTORY_SLOT_Y = 18.0f;

      private:
	void drawItems();
	void drawMouse(class Scene* scene);
	void close();
	void pickUp(class Scene* scene, EntityID item, std::size_t index);

	constexpr const static inline auto SIZE_KEY = "size";
	constexpr const static inline auto ITEMS_KEY = "items";
	constexpr const static inline auto COUNT_KEY = "count";

	constexpr const static inline auto INVENTORY_SPRITE_FILE = "ui/inventory.png";
};

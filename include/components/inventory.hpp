#pragma once

#include "components.hpp"
#include "managers/entityManager.hpp"
#include "opengl/shader.hpp"
#include "screens/screen.hpp"
#include "systems/renderSystem.hpp"
#include "third_party/rapidjson/document.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class Inventory : public Screen {
	friend class RenderSystem;

      public:
	explicit Inventory(class Game* game, std::size_t size, EntityID entity = 0);
	explicit Inventory(class Game* game, const rapidjson::Value& contents, EntityID entity);
	Inventory(Inventory&&) = delete;
	Inventory(const Inventory&) = delete;
	Inventory& operator=(Inventory&&) = delete;
	Inventory& operator=(const Inventory&) = delete;
	~Inventory() override = default;

	void load(const rapidjson::Document& contents);
	void save(rapidjson::Value& contents, rapidjson::Document::AllocatorType& allocator);

	virtual bool update(class Scene* scene, float delta) override;
	virtual void draw(class Scene* scene) override;
	// Sees if the entity can pick the item up, true if picked up false otherwise
	virtual bool tryPick(class Scene* scene, EntityID item);

      protected:
	constexpr const static inline auto REDISTRIBUTE = "inv_redistribute"_u;
	constexpr const static inline auto REDISTRIBUTE_ITEM = "inv_redistribute_item"_u;
	const EntityID mEntity;
	std::uint64_t mSize;
	std::vector<Components::Item> mItems;
	std::vector<std::uint64_t> mCount;

	std::uint64_t mLastClick;
	std::int64_t mLastClickPos;

	// Mouse traversal path but how tf do I get the inv??
	std::vector<std::pair<std::uint64_t, std::uint64_t>> mPath;
	// Unique ID for all classes
	template <typename T> [[nodiscard]] auto getID() {
		static std::uint64_t id = mCounter++;

		return id;
	}

	constexpr const static inline auto INVENTORY_TEXTURE_WIDTH = 176.0f;
	constexpr const static inline auto INVENTORY_TEXTURE_HEIGHT = 166.0f;
	constexpr const static inline auto INVENTORY_SLOTS_OFFSET_X = 7.5f;
	constexpr const static inline auto INVENTORY_SLOTS_OFFSET_Y = 8.5f;
	constexpr const static inline auto INVENTORY_INV_SCALE = 15.0f;
	constexpr const static inline auto INVENTORY_SLOT_X = 18.0f;
	constexpr const static inline auto INVENTORY_SLOT_Y = 18.0f;

	void handleKeys();

      private:
	void drawItems();
	void drawMouse(class Scene* scene);
	void close();
	void pickUp(class Scene* scene, EntityID item, std::size_t index);

	constexpr const static inline auto SIZE_KEY = "size";
	constexpr const static inline auto ITEMS_KEY = "items";
	constexpr const static inline auto COUNT_KEY = "count";

	constexpr const static inline auto INVENTORY_SPRITE_FILE = "ui/inventory.png";

	std::uint64_t mCounter;
};

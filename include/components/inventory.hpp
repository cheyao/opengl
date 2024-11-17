#pragma once

#include "managers/entityManager.hpp"
#include "screens/screen.hpp"
#include "third_party/json.hpp"

#include <cstddef>
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

      private:
	void close();

	constexpr const static inline char* const SIZE_KEY = "size";
	constexpr const static inline char* const CONTENTS_KEY = "contents";

	const EntityID mEntity;
	std::size_t mSize;
	std::vector<EntityID> mVector;
};

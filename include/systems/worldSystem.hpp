#pragma once

class WorldSystem {
      public:
	explicit WorldSystem(class Game* game);
	WorldSystem(WorldSystem&&) = delete;
	WorldSystem(const WorldSystem&) = delete;
	WorldSystem& operator=(WorldSystem&&) = delete;
	WorldSystem& operator=(const WorldSystem&) = delete;
	~WorldSystem() = default;

      private:
	void newBlock();

	class Game* mGame;
};

#include "components/playerInventory.hpp"

#include "game.hpp"
#include "managers/systemManager.hpp"
#include "systems/UISystem.hpp"

#include <SDL3/SDL.h>
#include <cstddef>

PlayerInventory::PlayerInventory(class Game* game, std::size_t size, EntityID entity)
	: Inventory(game, size, entity), mSelect(0) {}
PlayerInventory::PlayerInventory(class Game* game, const nlohmann::json& contents, EntityID entity)
	: Inventory(game, contents, entity), mSelect(0) {}

void PlayerInventory::update(class Scene*, float) {
	// Handle mouse and keys
	const auto keystate = mGame->getKeystate();

	// Quit menu
	if (keystate[SDL_SCANCODE_ESCAPE]) {
		mGame->getSystemManager()->getUISystem()->pop();
	}

	const auto select = [this, keystate](SDL_Scancode s, std::size_t n) {
		if (keystate[s]) {
			this->mSelect = n;
		}
	};

	select(SDL_SCANCODE_1, 0);
	select(SDL_SCANCODE_2, 1);
	select(SDL_SCANCODE_3, 2);
	select(SDL_SCANCODE_4, 3);
	select(SDL_SCANCODE_5, 4);
	select(SDL_SCANCODE_6, 5);
	select(SDL_SCANCODE_7, 6);
	select(SDL_SCANCODE_8, 7);
	select(SDL_SCANCODE_9, 8);
	select(SDL_SCANCODE_0, 9);
}

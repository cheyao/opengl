#include "systems/UISystem.hpp"

#include "game.hpp"
#include "opengl/mesh.hpp"
#include "scene.hpp"
#include "screens/screen.hpp"
#include <cstddef>

UISystem::UISystem() noexcept : mGame(Game::getInstance()), mMesh(nullptr) {
	constexpr const static float vertices[] = {
		0.0f, 0.0f, // TL
		0.0f, 1.0f, // BR
		1.0f, 0.0f, // TR
		1.0f, 1.0f  // BL
	};

	constexpr const static GLuint indices[] = {2, 1, 0,  // a
						   1, 2, 3}; // b

	mMesh.reset(new Mesh(vertices, {}, {}, indices, {}));
}

void UISystem::update(Scene* scene, const float delta) {
	if (mScreenStack.empty()) {
		return;
	}

	for (auto it = mScreenStack.rbegin(); it != mScreenStack.rend(); ++it) {
		if ((*it)->update(scene, delta)) {
			break;
		}
	}
}

void UISystem::draw(Scene* scene) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (const auto& screen : mScreenStack) {
		screen->draw(scene);
	}
}

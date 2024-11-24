#include "systems/renderSystem.hpp"

#include "components.hpp"
#include "components/inventory.hpp"
#include "game.hpp"
#include "managers/glManager.hpp"
#include "managers/shaderManager.hpp"
#include "managers/systemManager.hpp"
#include "managers/textureManager.hpp"
#include "misc/sparse_set_view.hpp"
#include "opengl/framebuffer.hpp"
#include "opengl/mesh.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/ubo.hpp"
#include "registers.hpp"
#include "scene.hpp"
#include "third_party/Eigen/Geometry"
#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <cstddef>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef IMGUI
#ifdef GLES
#define IMGUI_IMPL_OPENGL_ES3
#endif
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

// Hacks to get browser width and hight
EM_JS(int, browserHeight, (), { return window.innerHeight; });
EM_JS(int, browserWidth, (), { return window.innerWidth; });
#endif

// PERF: Use the other draw call
RenderSystem::RenderSystem(Game* game)
	: mGame(game), mWindow(nullptr), mCursor(nullptr), mGL(nullptr), mFramebuffer(nullptr), mMatricesUBO(nullptr),
	  mTextures(std::make_unique<TextureManager>(mGame->getBasePath())),
	  mShaders(std::make_unique<ShaderManager>(mGame->getBasePath())), mWidth(0), mHeight(0) {
	const SDL_DisplayMode* const DM = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

	SDL_Log("\n");
	SDL_Log("Display info:");
	SDL_Log("Display size: %dx%d", DM->w, DM->h);
	SDL_Log("Display format: %#010x", DM->format);
	SDL_Log("Refresh rate: %f", DM->refresh_rate);
	SDL_Log("\n");

	// Note: These must be set before the window is created https://wiki.libsdl.org/SDL3/SDL_GLattr
#ifdef GLES
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
	// Dunno why but apple want's this
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
#endif

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// NOTE: Don't set emscripten here, it breakes stuff, idk why
#ifdef ANDROID
	mWidth = DM->h;
	mHeight = DM->w;
#else
	mWidth = 1024;
	mHeight = 768;
#endif

	mWindow = SDL_CreateWindow("OpenGL", mWidth, mHeight,
				   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
#ifdef ANDROID
					   | SDL_WINDOW_FULLSCREEN
#endif
#ifdef HIDPI
					   | SDL_WINDOW_HIGH_PIXEL_DENSITY
#endif
	);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Game.cpp: Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with your system/SDL installation");

		throw std::runtime_error("Game.cpp: Failed to create SDL window");
	}

#ifdef __EMSCRIPTEN__
	mWidth = browserWidth();
	mHeight = browserHeight();

	SDL_SetWindowSize(mWindow, mWidth, mHeight);
#endif

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__)
	SDL_SetWindowMinimumSize(mWindow, 480, 320);
#endif

	mGL = std::make_unique<GLManager>(mWindow);

	SDL_assert(mGL->getContext() != nullptr);

#ifdef IMGUI
	SDL_Log("Initializing ImGUI");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
	io.IniFilename = nullptr;
#else
	// TODO: Storage path
#endif

	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#ifdef __ANDROID__
	io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
#endif
#ifdef DEBUG
	io.ConfigDebugIsDebuggerPresent = true;
#endif

	// Only give users light mode when their system are light
	// Why would anyone use light mode?
	switch (SDL_GetSystemTheme()) {
		case SDL_SYSTEM_THEME_LIGHT:
			ImGui::StyleColorsLight();

			break;

		case SDL_SYSTEM_THEME_UNKNOWN:
		case SDL_SYSTEM_THEME_DARK:
		default:
			ImGui::StyleColorsDark();

			break;
	}

	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
#ifdef GLES
	ImGui_ImplOpenGL3_Init("#version 300 es  ");
#else
	ImGui_ImplOpenGL3_Init("#version 410 core");
#endif

	SDL_Log("Finished Initializing ImGUI");
#endif

	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);

	mFramebuffer = std::make_unique<Framebuffer>(this);
	// NOTE: Uncomment if testing framebuffer module
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Matrix uniform
	mMatricesUBO = std::make_unique<UBO>(2 * sizeof(Eigen::Affine3f));
	mMatricesUBO->bind(0);

	// Debug Info
	mGL->printInfo();

	setOrtho();

	constexpr const static float vertices[] = {
		0.0f, 0.0f, 0.0f, // TL
		0.0f, 1.0f, 0.0f, // BR
		1.0f, 0.0f, 0.0f, // TR
		1.0f, 1.0f, 0.0f  // BL
	};

	constexpr const static float texturePos[] = {
		0.0f, 1.0f, // TR
		0.0f, 0.0f, // BR
		1.0f, 1.0f, // TL
		1.0f, 0.0f  // BL
	};

	const static GLuint indices[] = {2, 1, 0,  // a
					 1, 2, 3}; // b

	mMesh = std::unique_ptr<Mesh>(new Mesh(vertices, {}, texturePos, indices, {}));

#ifndef __ANDROID__
	SDL_Surface* cursorSurface = SDL_LoadBMP((mGame->getBasePath() + "assets/textures/crosshair.bmp").data());

	if (cursorSurface) {
		mCursor = SDL_CreateColorCursor(cursorSurface, 8, 8);

		SDL_DestroySurface(cursorSurface);

		if (mCursor && SDL_SetCursor(mCursor)) {
			SDL_Log("\033[32mSuccesfully set cursor\033[0m");
		} else {
			SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\033[31mFailed to set cursor: %s\033[0m", SDL_GetError());
		}
	} else {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "\033[31mFailed to get cursor surface: %s\033[0m", SDL_GetError());
	}
#endif
}

RenderSystem::~RenderSystem() {
	SDL_DestroyWindow(mWindow);
	SDL_DestroyCursor(mCursor);
}

void RenderSystem::setDemensions(int width, int height) {
	mWidth = width;
	mHeight = height;

	int w, h;
	SDL_GetWindowSizeInPixels(mWindow, &w, &h);

	glViewport(0, 0, w, h);
	mFramebuffer->setDemensions(w, h);

	setOrtho();
}

void RenderSystem::draw(Scene* scene) {
	glClearColor(0.470588235294f, 0.65490190784f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);

	const Eigen::Vector2f cameraOffset = -scene->get<Components::position>(mGame->getPlayerID()).mPosition +
					     Eigen::Vector2f(mWidth, mHeight) / 2;

	// Draw blocks
	Shader* shader = this->getShader("block.vert", "block.frag");
	shader->activate();
	shader->set("size"_u, (float)Components::block::BLOCK_SIZE, (float)Components::block::BLOCK_SIZE);
	shader->set("texture_diffuse"_u, 0);
	shader->set("offset"_u, cameraOffset);
	shader->set("scale"_u, 1.0f);

	// PERF: Use some lists to send the data
	for (const auto& [entity, texture, block] : scene->view<Components::texture, Components::block>().each()) {
		if (!block.mBreak) {
			continue;
		}

		shader->set("position"_u, block.mPosition);

		texture.mTexture->activate(0);

		mMesh->draw(shader);

		scene->get<Components::block>(entity).mBreak = false;
	}

	// Draw other textures
	shader->set("position"_u, 0, 0);
	for (const auto& [_, texture, position] : scene->view<Components::texture, Components::position>().each()) {
		const Eigen::Vector2f offset = position.mPosition + cameraOffset;
		shader->set("offset"_u, offset);
		shader->set("size"_u, texture.mTexture->getSize());
		shader->set("scale"_u, texture.mScale);

		texture.mTexture->activate(0);

		mMesh->draw(shader);
	}

	drawHUD(scene);

#if defined(IMGUI) && !defined(GLES)
	static bool hitbox = false;
	static bool vector = false;

	ImGui::Begin("Main menu");
	ImGui::Checkbox("Show hitboxes", &hitbox);
	ImGui::Checkbox("Show velocity vectors", &vector);
	ImGui::End();

	// Debug layer rendering
	if (scene->getSignal("collisionEditor"_u) || hitbox) {
		GLint mode[2];
		if (hitbox && glPolygonMode != nullptr) {
			glGetIntegerv(GL_POLYGON_MODE, mode);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		Shader* editorShader = mGame->getSystemManager()->getShader("block.vert", "editor.frag");

		editorShader->activate();
		editorShader->set("wireframe"_u, hitbox);

		for (const auto& [_, collision, position] :
		     scene->view<Components::collision, Components::position>().each()) {
			const Eigen::Vector2f offset = position.mPosition + collision.mOffset + cameraOffset;

			editorShader->set("offset"_u, offset);
			editorShader->set("size"_u, collision.mSize);

			mMesh->draw(editorShader);
		}

		if (hitbox && glPolygonMode != nullptr) {
			glPolygonMode(GL_FRONT_AND_BACK, mode[0]);
		}
	}

	if (vector) {
		// See https://stackoverflow.com/questions/3484260/opengl-line-width
		static float magnitude = 1.0f;
		ImGui::Begin("Main menu");
		ImGui::SliderFloat("magnitude", &magnitude, 0, 3.0f);
		ImGui::End();

		Shader* vectorShader =
			mGame->getSystemManager()->getShader("vector.vert", "vector.frag", "vector.geom");

		vectorShader->activate();
		vectorShader->set("magnitude"_u, magnitude);

		for (const auto& [_, velocity, position, texture] :
		     scene->view<Components::velocity, Components::position, Components::texture>().each()) {
			if (nearZero(velocity.mVelocity.x() + velocity.mVelocity.y())) {
				continue;
			}

			Eigen::Affine3f model = Eigen::Affine3f::Identity();

			model.translate((Eigen::Vector3f() << (position.mPosition), 0.0f).finished());

			vectorShader->set("model"_u, model);
			vectorShader->set("size"_u,
					  Eigen::Vector2f(texture.mTexture->getWidth(), texture.mTexture->getHeight()));
			const Eigen::Vector2f center = position.mPosition + texture.mTexture->getSize() / 2;
			vectorShader->set("position"_u, center);
			vectorShader->set("velocity"_u, velocity.mVelocity);

			mMesh->draw(vectorShader);
		}
	}

	mShaders->debugGui();
#endif

#ifdef __EMSCRIPTEN__
	// Emscripten, SDL3 doesn't correctly report resize atm
	if (browserWidth() != mWidth || browserHeight() != mHeight) {
		mWidth = browserWidth();
		mHeight = browserHeight();

		SDL_SetWindowSize(mWindow, mWidth, mHeight);
	}
#endif
}

void RenderSystem::present() const { mFramebuffer->swap(); }

void RenderSystem::swapWindow() const { SDL_GL_SwapWindow(mWindow); }

Texture* RenderSystem::getTexture(const std::string& name, const bool srgb) { return mTextures->get(name, srgb); }

Shader* RenderSystem::getShader(const std::string& vert, const std::string& frag, const std::string& geom) {
	return mShaders->get(vert, frag, geom);
}

void RenderSystem::setPersp() const {
	constexpr const static float near = 0.1f;
	constexpr const static float far = 100.0f;
	constexpr const static float range = far - near;
	constexpr const static float fov = 45.0f;
	constexpr const static float theta = fov * 0.5;
	const static float invtan = 1.0f / SDL_tan(theta);

	const float aspect = static_cast<float>(mWidth) / mHeight;

	// https://www.songho.ca/opengl/gl_projectionmatrix.html
	Eigen::Affine3f projectionMatrix = Eigen::Affine3f::Identity();
	projectionMatrix(0, 0) = invtan / aspect;
	projectionMatrix(1, 1) = invtan;
	projectionMatrix(2, 2) = -(near + far) / range;
	projectionMatrix(3, 2) = -1;
	projectionMatrix(2, 3) = -2 * near * far / range;
	projectionMatrix(3, 3) = 0;

	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), projectionMatrix);
}

void RenderSystem::drawHUD(Scene* scene) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SystemManager* systemManager = mGame->getSystemManager();
	Texture* texture = systemManager->getTexture("ui/hotbar.png");
	Shader* shader = systemManager->getShader("ui.vert", "ui.frag");
	const Eigen::Vector2f dimensions = systemManager->getDemensions();

	shader->activate();
	shader->set("texture_diffuse"_u, 0);
	texture->activate(0);

	float x;
	float y;
	float scale;

	if (dimensions.x() <= dimensions.y()) {
		x = dimensions.x() / 5 * 4;
		y = x / texture->getWidth() * texture->getHeight();

		scale = x / texture->getWidth();
	} else {
		y = dimensions.y() / 8 * 1;
		x = y / texture->getHeight() * texture->getWidth();

		scale = y / texture->getHeight();
	}
	(void)scale;

	Eigen::Vector2f offset = Eigen::Vector2f((dimensions.x() - x) / 2, 0.0f);

	shader->set("size"_u, x, y);
	shader->set("offset"_u, offset);

	mMesh->draw(shader);

	//Texture* selectTexture = systemManager->getTexture("ui/hotbar_selection.png");

	glDisable(GL_BLEND);

	float size = y / 2;
	shader->set("size"_u, size, size);

	Inventory* inventory = scene->get<Components::inventory>(mGame->getPlayerID()).mInventory;
	for (std::size_t i = 0; i < 9; ++i) {
		if (inventory->mCount[i] == 0) {
			continue;
		}

		Texture* itemTexture = systemManager->getTexture(registers::TEXTURES.at(inventory->mItems[i]));
		itemTexture->activate(0);

		shader->set("offset"_u, offset.x() + i * x / 9 + y / 4, y / 4);

		mMesh->draw(shader);

		if (inventory->mCount[i] > 1) {
			// Adjust the offset according to number of digits
			float yoffset = inventory->mCount[i] >= 10 ? (y / 5 * 3) : (y / 4 * 3);
			mGame->getSystemManager()->getTextSystem()->draw(
				std::to_string(inventory->mCount[i]),
				Eigen::Vector2f(offset.x() + i * x / 9 + yoffset, y / 9), false);
		}

		shader->activate();
	}
}

void RenderSystem::setOrtho() const {
	constexpr const static float left = 0.0f;
	constexpr const static float bottom = 0.0f;
	constexpr const static float near = 0.0f;
	constexpr const static float far = 1000.0f;

	const float right = static_cast<float>(mWidth);
	const float top = static_cast<float>(mHeight);

	Eigen::Affine3f projectionMatrix = Eigen::Affine3f::Identity();
	projectionMatrix(0, 0) = 2.0f / (right - left);
	projectionMatrix(1, 1) = 2.0f / (top - bottom);
	projectionMatrix(2, 2) = -2.0f / (far - near);
	projectionMatrix(3, 3) = 1;
	projectionMatrix(0, 3) = -(right + left) / (right - left);
	projectionMatrix(1, 3) = -(top + bottom) / (top - bottom);
	projectionMatrix(2, 3) = -(far + near) / (far - near);

	mMatricesUBO->set(0 * sizeof(Eigen::Affine3f), projectionMatrix);
}

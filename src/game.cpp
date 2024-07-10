#include "game.hpp"

#include "opengl/camera.hpp"
#include "opengl/glmanager.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/vertexArray.hpp"
#include "utils.hpp"

#include <third_party/Eigen/Dense>
#include <third_party/glad/glad.h>

#include <SDL3/SDL.h>
#include <string>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

EM_JS(int, browserHeight, (), { return window.innerHeight; });
EM_JS(int, browserWidth, (), { return window.innerWidth; });
EM_JS(int, canvasResize, (), {
	canvas.width = window.innerWidth;
	canvas.height = window.innerHeight;
});
#endif

Game::Game()
	: mWindow(nullptr), mGL(nullptr), mShader(nullptr), mVertex(nullptr), mTicks(0),
	  mBasePath(), mBox(nullptr), mFace(nullptr), mCamera(nullptr), mWidth(0),
	  mHeight(0) {
	mWindow = SDL_CreateWindow("Golf", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with "
				  "your SDL installation");

		throw 1;
	}
	SDL_SetWindowMinimumSize(mWindow, 480, 320);

	mGL = new GLManager(mWindow);

	mGL->printInfo();

	// Set size (For android)
#ifdef __EMSCRIPTEN__
	mHeight = browserHeight();
	mWidth = browserWidth();
	canvasResize();
	SDL_SetWindowSize(mWindow, mWidth, mHeight);
#else
	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);
#endif

	glViewport(0, 0, mWidth, mHeight);

#ifdef IMGUI
	// Init ImGUI
	SDL_Log("Initializing ImGUI");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
#ifdef __EMSCRIPTEN__
	io.IniFilename = nullptr;
#endif

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGL->getContext());
	ImGui_ImplOpenGL3_Init("#version 400");

	SDL_Log("Finished Initializing ImGUI");
#endif

	char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
		SDL_free(basepath); // We gotta free da pointer UwU
	}

	// TODO: Get a icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_DestroySurface(icon);
	*/

	mTicks = SDL_GetTicks();

	setup();
}

void Game::setup() {
	SDL_Log("Setting up game");

	glEnable(GL_DEPTH_TEST);

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	unsigned int indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, };

	mVertex = new VertexArray(vertices, sizeof(vertices) / sizeof(vertices[0]), indices,
							  sizeof(indices) / sizeof(indices[0]));
	mShader = new Shader(fullPath("shaders/basic.vert"), fullPath("shaders/basic.frag"));
	mShader->activate();
	mShader->set("box", 0);
	mShader->set("face", 1);

	mBox = new Texture(fullPath("container.png"));
	mFace = new Texture(fullPath("face.png"), true);

	mCamera = new Camera();
	mCamera->project(toRadians(45.0f), mWidth, mHeight, 0.1f, 100.0f);
	mCamera->view(Eigen::Vector3f(0.0f, 0.0f, 3.0f), Eigen::Vector3f(0.0f, 0.0f, 0.0f),
				  Eigen::Vector3f(0.0f, 1.0f, 0.0f));

	SDL_Log("Successfully initialized OpenGL and game\n");
}

int Game::iterate() {
#ifdef __EMSCRIPTEN__
	// Web hack
	SDL_SetWindowSize(mWindow, browserWidth(), browserHeight());
#endif

	input();
	update();
	gui();
	draw();

	return 0;
}

void Game::input() {
	const Uint8* keys = SDL_GetKeyboardState(nullptr);

	(void) keys;
}

void Game::update() {
	// Update the game
	float delta = (SDL_GetTicks() - mTicks) / 1000.0f;
	if (delta > 0.05) {
		delta = 0.05;
	}
	mTicks = SDL_GetTicks();
}

#ifdef IMGUI
bool demoMenu = false;
bool vsync = true;
bool wireframe = false;
void Game::gui() {
	// Update ImGui Frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", (1000.f / io.Framerate), io.Framerate);
		ImGui::Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices,
					io.MetricsRenderIndices, io.MetricsRenderIndices / 3);

		// ImGui::Checkbox("Debug", &debugMenu);
		ImGui::Checkbox("Demo", &demoMenu);
		ImGui::Checkbox("VSync", &vsync);
		ImGui::Checkbox("Wireframe", &wireframe);

		ImGui::End();
	}

	if (demoMenu) {
		ImGui::ShowDemoWindow(&demoMenu);
	}

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	SDL_GL_SetSwapInterval(vsync);
}
#else
void Game::gui() {}
#endif

void Game::draw() {
#ifdef IMGUI
	ImGui::Render();
#endif
	const float radius = 10.0f;
	float x = sin(static_cast<float>(SDL_GetTicks()) / 1000) * radius;
	float z = cos(static_cast<float>(SDL_GetTicks()) / 1000) * radius;
	mCamera->view(Eigen::Vector3f(x, 0.0f, z), 
			      Eigen::Vector3f(0.0f, 0.0f, 0.0f),
				  Eigen::Vector3f(0.0f, 1.0f, 0.0f));

	Eigen::Vector3f cubePositions[] = {
		Eigen::Vector3f( 0.0f,  0.0f,  0.0f), 
		Eigen::Vector3f( 2.0f,  5.0f, -15.0f), 
		Eigen::Vector3f(-1.5f, -2.2f, -2.5f),  
		Eigen::Vector3f(-3.8f, -2.0f, -12.3f),  
		Eigen::Vector3f( 2.4f, -0.4f, -3.5f),  
		Eigen::Vector3f(-1.7f,  3.0f, -7.5f),  
		Eigen::Vector3f( 1.3f, -2.0f, -2.5f),  
		Eigen::Vector3f( 1.5f,  2.0f, -2.5f), 
		Eigen::Vector3f( 1.5f,  0.2f, -1.5f), 
		Eigen::Vector3f(-1.3f,  1.0f, -1.5f)  
	};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShader->activate();
	mVertex->activate();

	mBox->activate(0);
	mFace->activate(1);

	mShader->set("view", mCamera->mViewMatrix);
	mShader->set("proj", mCamera->mProjectionMatrix);

	for (auto i = 0; i < 10; ++i) {
		Eigen::Affine3f modelMat = Eigen::Affine3f::Identity();
		modelMat.translate(cubePositions[i]);
		if (i % 3 == 0) {
			modelMat.rotate(Eigen::AngleAxisf(static_cast<float>(SDL_GetTicks()) / 1000, Eigen::Vector3f(0.5f, 1.0f, 0.0f).normalized()));
		}
		mShader->set("model", modelMat);
		glDrawElements(GL_TRIANGLES, mVertex->indices(), GL_UNSIGNED_INT, 0);
	}

#ifdef IMGUI
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

	SDL_GL_SwapWindow(mWindow);
}

int Game::event(const SDL_Event& event) {
#ifdef IMGUI
	ImGui_ImplSDL3_ProcessEvent(&event);
#endif

	switch (event.type) {
		case SDL_EVENT_QUIT: {
			return 1;
		}

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
			if (event.window.windowID == SDL_GetWindowID(mWindow)) {
				return 1;
			}

			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			mWidth = event.window.data1;
			mHeight = event.window.data2;
			glViewport(0, 0, event.window.data1, event.window.data2);
			mCamera->project(toRadians(45.0f), mWidth, mHeight, 0.1f, 100.0f);
			break;
		}
	}

	return 0;
}

Game::~Game() {
	SDL_Log("Quitting game\n");

#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	delete mShader;
	delete mVertex;
	delete mBox;
	delete mFace;
	delete mGL;

	SDL_DestroyWindow(mWindow);
}

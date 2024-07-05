#include "game.hpp"

#include "common.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertexArray.hpp"

#include <glad/glad.h>

#include <SDL3/SDL.h>

#ifdef IMGUI
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#endif

#include <string>

Game::Game() : mWindow(nullptr), mContext(nullptr), mShader(nullptr), mVertex(nullptr), mTicks(0), mBasePath(), mBox(nullptr), mFace(nullptr), mixer(0.2) {};
// TODO: RAII

void printGLInfo() {
	SDL_Log("Vendor   : %s\n", glGetString(GL_VENDOR));
	SDL_Log("Renderer : %s\n", glGetString(GL_RENDERER));
	SDL_Log("Version  : %s\n", glGetString(GL_VERSION));
	SDL_Log("GLSL     : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	int maj;
	int min;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min);
	SDL_Log("Context  : %d.%d\n", maj, min);

	glGetIntegerv(GL_MAJOR_VERSION, &maj);
	glGetIntegerv(GL_MINOR_VERSION, &min);
	SDL_Log("Context  : %d.%d\n", maj, min);

	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	SDL_Log("Maximum number of vertex attributes supported: %d\n", nrAttributes);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nrAttributes);
	SDL_Log("Maximum number of texture units supported: %d\n", nrAttributes);
}

int Game::init() {
	mWindow = SDL_CreateWindow("Golf", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to make SDL window, there is something wrong with "
				  "your SDL installation");

		return 1;
	}

	mContext = SDL_GL_CreateContext(mWindow);
	if (mWindow == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize OpenGL Context, there is something "
				  "wrong with your OpenGL");
		return 1;
	}

	// TODO: Get a icon
	/*
	SDL_Surface *icon = IMG_Load("assets/icon.png");
	SDL_SetWindowIcon(mWindow, icon);
	SDL_DestroySurface(icon);
	*/

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

#ifndef __APPLE__
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to init glad!\n");
		ERROR_BOX("Failed to initialize GLAD, there is something wrong with your OpenGL");
	}
#endif

	// Set VSync
	SDL_GL_SetSwapInterval(1);

	printGLInfo();

	glViewport(0, 0, 1024, 768);

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

	ImGui_ImplSDL3_InitForOpenGL(mWindow, mContext);
	ImGui_ImplOpenGL3_Init("#version 400");

	SDL_Log("Finished Initializing ImGUI");
#endif

	char* basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath = std::string(basepath);
		SDL_free(basepath); // We gotta free da pointer UwU
	}

	mTicks = SDL_GetTicks();

	return setup();
}

int Game::setup() {
	float vertices[] = {
		// positions  // colors   // texture coords
		0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
		0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
		-0.5f, 0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f	// top left
	};
	unsigned int indices[] = {0, 1, 3, 1, 2, 3};

	mVertex = new VertexArray(vertices, sizeof(vertices) / sizeof(vertices[0]), indices,
							  sizeof(indices) / sizeof(indices[0]));
	mShader = new Shader(fullPath("shaders/basic.vert"), fullPath("shaders/basic.frag"));
	mShader->activate();
	mShader->set("box", 0);
	mShader->set("face", 1);

	mBox = new Texture(fullPath("container.png"));
	mFace = new Texture(fullPath("face.png"), true);

	SDL_Log("Successfully initialized OpenGL and game\n");

	return 0;
}

int Game::iterate() {
	// Loop
	input();
	update();
	gui();
	draw();

	return 0;
}

void Game::input() {
	const Uint8* keys = SDL_GetKeyboardState(nullptr);

	if (keys[SDL_SCANCODE_UP]) {
		mixer += 0.1;
	} else if (keys[SDL_SCANCODE_DOWN]) {
		mixer -= 0.1;
	}
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
#endif

void Game::gui() {
#ifdef IMGUI
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
#endif
}

void Game::draw() {
#ifdef IMGUI
	ImGui::Render();
#endif

	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	mVertex->activate();

	mBox->activate(0);
	mFace->activate(1);

	mShader->set("mixer", mixer);

	// FIXME: Hardcoded triangle count
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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

		case SDL_EVENT_KEY_DOWN: {
			if (event.key.key == SDLK_ESCAPE) {
				return 1;
			}

			break;
		}

		case SDL_EVENT_WINDOW_RESIZED: {
			if (event.window.data1 < 720 || event.window.data2 < 480) {
				SDL_SetWindowSize(mWindow, event.window.data1 < 720 ? 720 : event.window.data1,
								  event.window.data2 < 480 ? 480 : event.window.data2);
				glViewport(0, 0, event.window.data1 < 720 ? 720 : event.window.data1,
						   event.window.data2 < 480 ? 480 : event.window.data2);
			} else {
				glViewport(0, 0, event.window.data1, event.window.data2);
			}

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

	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
}

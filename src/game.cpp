#include "game.hpp"
#include "common.hpp"

#include <glad/glad.h>

#include <SDL3/SDL.h>

#ifdef IMGUI
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#endif

#include <string>

Game::Game()
	: mWindow(nullptr), mContext(nullptr), mTicks(0), mBasePath(""), mVAO(0), mShaderProgram(0){};

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

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to init glad!\n");
		ERROR_BOX("Failed to initialize GLAD, there is something wrong with your OpenGL");
	}

	SDL_Log("Vendor   : %s", glGetString(GL_VENDOR));
	SDL_Log("Renderer : %s", glGetString(GL_RENDERER));
	SDL_Log("Version  : %s", glGetString(GL_VERSION));
	SDL_Log("GLSL     : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	int maj;
	int min;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min);
	SDL_Log("Context  : %d.%d", maj, min);

	glGetIntegerv(GL_MAJOR_VERSION, &maj);
	glGetIntegerv(GL_MINOR_VERSION, &min);
	SDL_Log("Context  : %d.%d", maj, min);

	// Set VSync
	SDL_GL_SetSwapInterval(1);

	glViewport(0, 0, 1024, 768);

#ifdef IMGUI
	// Init ImGUI
	SDL_Log("Initializing ImGUI");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
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

	mTicks = SDL_GetTicks();

	char *basepath = SDL_GetBasePath();
	if (basepath != nullptr) {
		mBasePath += basepath;
		SDL_free(basepath); // We gotta free da pointer UwU
	}

	glGenVertexArrays(1, &mVAO);

	// Init OpenGL part 2
	float triangle[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	const char *vertexShaderSource =
		R""(
#version 400 core
layout (location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}

)"";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Error checking
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile vertex shader: %s\n", infoLog);
		ERROR_BOX("Failed to compile vertex shader");
		return 1;
	}

	const char *fragmentShaderSource =
		R""(
#version 400 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
)"";
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Error checking #2
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile fragment shader: %s\n", infoLog);
		ERROR_BOX("Failed to compile fragment shader");
		return 1;
	}

	mShaderProgram = glCreateProgram();
	glAttachShader(mShaderProgram, vertexShader);
	glAttachShader(mShaderProgram, fragmentShader);
	glLinkProgram(mShaderProgram);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(mShaderProgram, 512, NULL, infoLog);
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to compile link shader: %s\n", infoLog);
		ERROR_BOX("Failed to link shader");
		return 1;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Save the conf to VAO
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<GLvoid *>(0));
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

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
	// TODO:
	const Uint8 *keys = SDL_GetKeyboardState(nullptr);

	(void)keys;
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
bool statisticsMenu = false;
#endif

void Game::gui() {
#ifdef IMGUI
	// Update ImGui Frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	/* Main menu */ {
		ImGui::Begin("Main menu");

		ImGui::Checkbox("Statistics", &statisticsMenu);
		// ImGui::Checkbox("Debug", &debugMenu);
		ImGui::Checkbox("Demo", &demoMenu);

		ImGui::End();
	}

	if (demoMenu) {
		ImGui::ShowDemoWindow(&demoMenu);
	}

	if (statisticsMenu) {
		ImGui::Begin("Statistics", &statisticsMenu);

		ImGuiIO &io = ImGui::GetIO();
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", (1000.f / io.Framerate), io.Framerate);
		ImGui::Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices, io.MetricsRenderIndices / 3);

		ImGui::End();
	}
#endif
}

void Game::draw() {
#ifdef IMGUI
	ImGui::Render();
#endif

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(mShaderProgram);
	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

#ifdef IMGUI
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

	SDL_GL_SwapWindow(mWindow);
}

int Game::event(const SDL_Event &event) {
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

	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
}

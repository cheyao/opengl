#include "managers/glManager.hpp"

#include "third_party/glad/glad.h"
#include "utils.hpp"

#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>

GLManager::GLManager() : mContext(nullptr) {
#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
}

void GLManager::bindContext(SDL_Window* window) {
	mContext = SDL_GL_CreateContext(window);
	if (mContext == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Failed to create window: %s\n", SDL_GetError());
		ERROR_BOX("Failed to initialize OpenGL Context, there is something "
				  "wrong with your OpenGL");

		throw std::runtime_error("glManager.cpp: Failed to get opengl context");
	}

#ifdef GLES
	if (gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
#else
	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
#endif
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to init glad!\n");
		ERROR_BOX("Failed to initialize GLAD, there is something wrong with your OpenGL");

		throw std::runtime_error("glManager.cpp: Failed to init glad");
	}

	SDL_GL_MakeCurrent(window, mContext);

	SDL_GL_SetSwapInterval(1);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLManager::~GLManager() { SDL_GL_DestroyContext(mContext); }

void GLManager::printInfo() {
	SDL_Log("Video driver: %s\n", SDL_GetCurrentVideoDriver());
	SDL_Log("Vendor      : %s\n", glGetString(GL_VENDOR));
	SDL_Log("Renderer    : %s\n", glGetString(GL_RENDERER));
	SDL_Log("Version     : %s\n", glGetString(GL_VERSION));
	SDL_Log("GLSL        : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint n = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);

	std::string extensions;
	for (GLint i = 0; i < n; ++i) {
		extensions += std::string(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
		extensions += ' ';
	}
	SDL_Log("Extensions  : %s\n", extensions.data());

	GLint maj = 0;
	GLint min = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min);
	SDL_Log("Context     : %d.%d\n", maj, min);

	glGetIntegerv(GL_MAJOR_VERSION, &maj);
	glGetIntegerv(GL_MINOR_VERSION, &min);
	SDL_Log("OpenGL      : %d.%d\n", maj, min);

	SDL_Log("\n");

	GLint value = 0;

	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
	SDL_Log("SDL_GL_RED_SIZE: requested 8, got %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
	SDL_Log("SDL_GL_GREEN_SIZE: requested 8, got %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
	SDL_Log("SDL_GL_BLUE_SIZE: requested 8, got %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value);
	SDL_Log("SDL_GL_STENCIL_SIZE: requested 8, got %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
	SDL_Log("SDL_GL_DEPTH_SIZE: requested 24, got %d\n", value);

	SDL_Log("\n");

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
	SDL_Log("Maximum number of vertex attributes supported: %d\n", value);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &value);
	SDL_Log("Maximum number of texture units supported: %d\n", value);
}

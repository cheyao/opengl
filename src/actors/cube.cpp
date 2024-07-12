#include "actors/cube.hpp"

#include "game.hpp"

const float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
						  0.5f,	 0.5f,	-0.5f, 1.0f, 1.0f, 0.5f,  0.5f,	 -0.5f, 1.0f, 1.0f,
						  -0.5f, 0.5f,	-0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

						  -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,	1.0f, 0.0f,
						  0.5f,	 0.5f,	0.5f,  1.0f, 1.0f, 0.5f,  0.5f,	 0.5f,	1.0f, 1.0f,
						  -0.5f, 0.5f,	0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,	0.0f, 0.0f,

						  -0.5f, 0.5f,	0.5f,  1.0f, 0.0f, -0.5f, 0.5f,	 -0.5f, 1.0f, 1.0f,
						  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
						  -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,	 0.5f,	1.0f, 0.0f,

						  0.5f,	 0.5f,	0.5f,  1.0f, 0.0f, 0.5f,  0.5f,	 -0.5f, 1.0f, 1.0f,
						  0.5f,	 -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
						  0.5f,	 -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,	 0.5f,	1.0f, 0.0f,

						  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
						  0.5f,	 -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,	1.0f, 0.0f,
						  -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

						  -0.5f, 0.5f,	-0.5f, 0.0f, 1.0f, 0.5f,  0.5f,	 -0.5f, 1.0f, 1.0f,
						  0.5f,	 0.5f,	0.5f,  1.0f, 0.0f, 0.5f,  0.5f,	 0.5f,	1.0f, 0.0f,
						  -0.5f, 0.5f,	0.5f,  0.0f, 0.0f, -0.5f, 0.5f,	 -0.5f, 0.0f, 1.0f};
const static unsigned int indices[] = {
	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
};

Cube::Cube(Game* game) : Actor(game) {
	mVertex = new VertexArray(vertices, sizeof(vertices) / sizeof(vertices[0]), indices,
							  sizeof(indices) / sizeof(indices[0]));
}

Cube::~Cube() {
	delete mVertex;
}

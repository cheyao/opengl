#version 400 core

layout (location = 0) in vec3 aPos;

out vec3 texPos;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	texPos = aPos;

	gl_Position = proj * view */* model**/ vec4(aPos, 1.0f);
}

#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 model;
uniform mat4 proj;

void main() {
	gl_Position = proj * model * vec4(aPos, 1.0f);
	vTexPos = aTexPos;
	// TODO: Maybe deduce tex pos from aPos
}

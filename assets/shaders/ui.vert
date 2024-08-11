#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform mat4 model;

void main() {
	gl_Position = model * vec4(aPos, 1.0f);
	// TODO: Maybe deduce tex pos from aPos
	vTexPos = aTexPos;
}

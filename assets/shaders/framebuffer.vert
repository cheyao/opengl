#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 texPos;

void main() {
	gl_Position = vec4(aPos, 1.0f);
	texPos = aTexPos;
}

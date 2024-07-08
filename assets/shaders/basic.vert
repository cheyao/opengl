#version 400 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 inTexPos;

out vec2 texPos;

uniform mat4 trans;

void main() {
    gl_Position = trans * vec4(pos, 0.0, 1.0f);
	texPos = inTexPos;
}

#version 400 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 inTexPos;

out vec3 dstColor;
out vec2 texPos;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    dstColor = color;
	texPos = inTexPos;
}

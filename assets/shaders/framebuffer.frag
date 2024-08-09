#version 410 core
precision mediump float;

in vec2 texPos;

layout (location = 0) out vec4 color;

uniform sampler2D screen;

void main() {
	color = vec4(vec3(texture(screen, texPos)), 1.0f);
} 

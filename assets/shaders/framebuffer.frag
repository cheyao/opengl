#version 410 core
precision mediump float;

in vec2 texPos;

layout (location = 0) out vec4 color;

uniform sampler2D screen;

void main() {
	const float gamma = 2.2;

	color = vec4(pow(vec3(texture(screen, texPos)), vec3(1.0f / gamma)), 1.0f);
	// color = vec4(vec3(texture(screen, texPos)), 1.0f);
} 

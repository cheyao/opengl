#version 410 core
precision mediump float;

in vec2 texPos;

layout (location = 0) out vec4 color;

// Getting rid of a error
layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform sampler2D screen;

void main() {
#ifdef GL_ES
	color = vec4(vec3(texture(screen, texPos)), 1.0f);
#else
	const float gammaVal = 2.2;
	color = vec4(pow(vec3(texture(screen, texPos)), vec3(1.0f / gammaVal)), 1.0f);
#endif
} 

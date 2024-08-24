#version 410 core
precision mediump float;

in vec2 vTexPos;

layout (location = 0) out vec4 color;

uniform sampler2D texture_diffuse;

void main() {
	color = texture(texture_diffuse, vTexPos);
} 

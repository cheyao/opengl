#version 410 core
precision mediump float;

layout (location = 0) out vec4 color;

in vec2 vTexPos;

uniform sampler2D texture_diffuse0;

void main() {
	color = texture(texture_diffuse0, vTexPos);
} 

#version 410 core
precision mediump float;

in vec3 texPos;

layout (location = 0) out vec4 color;

uniform samplerCube texture_diffuse0;

void main() {
	color = texture(texture_diffuse0, texPos);
}


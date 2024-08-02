#version 400 core
precision mediump float;

in vec3 texPos;

out vec4 color;

uniform samplerCube texture_diffuse0;

void main() {
	color = texture(texture_diffuse0, texPos);
}


#version 400 core

in vec2 texPos;

out vec4 color;

uniform sampler2D texture_diffuse0;

void main() {
	color = texture(texture_diffuse0, texPos);
}


#version 410 core
precision mediump float;

in VS_OUT {
	vec2 texPos;
} fs_in;

out vec4 color;

uniform sampler2D texture_diffuse0;

void main() {
	color = texture(texture_diffuse0, fs_in.texPos);
}


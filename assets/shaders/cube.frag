#version 410 core
precision mediump float;

uniform sampler2D texture_diffuse0;

in VS_OUT {
	vec2 texPos;
} fs_in;

layout (location = 0) out vec4 color;

void main() {
    color = texture(texture_diffuse0, fs_in.texPos);
}

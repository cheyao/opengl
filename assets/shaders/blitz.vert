#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

uniform vec2 offset;
uniform int position;
uniform sampler2D texture_diffuse;

void main() {
	vec2 offset = vec2(position % 64 * 16, position / 64 * 16);
	vec2 pos = aPos * vec2(textureSize(texture_diffuse, 0));
	gl_Position = vec4(pos, 0.0f, 1.0f);

	vTexPos = aTexPos;
}

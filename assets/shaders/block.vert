#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
};
uniform vec2 offset;
uniform vec2 size;
uniform float scale;

uniform ivec2 position;

void main() {
	vec2 pos = (aPos * scale + position) * size + offset;
	gl_Position = proj * vec4(pos, 0.0f, 1.0f);

	vTexPos = aTexPos;
}

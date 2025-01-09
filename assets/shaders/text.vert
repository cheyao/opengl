#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform vec2 offset;
uniform vec2 size;

void main() {
	gl_Position = proj * vec4(offset + vec2(aPos.x * size.x, aPos.y * size.y), 0.0f, 1.0f);

	vTexPos = aTexPos;
}

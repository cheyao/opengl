#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
// uniform mat4 model;
uniform vec2 offset;
uniform vec2 size;

uniform ivec2 position;

void main() {
	vec2 pos = vec2(aPos.x * size.x, aPos.y * size.y) + offset;
	pos.x += float(position.x) * size.x;
	pos.y += float(position.y) * size.y;
	gl_Position = proj * vec4(pos, 0.0f, 1.0f);

	vTexPos = aTexPos;
}

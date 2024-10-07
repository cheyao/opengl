#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform mat4 model;
uniform vec2 size;

uniform ivec2 position;

void main() {
	vec2 pos = vec2(model * vec4(aPos.x * size.x, aPos.y * size.y, 0.0f, 1.0f));
	pos.x += /position.x * 32.0f;
	gl_Position = proj * vec4(pos, 0.0f, 1.0f);

	vTexPos = aTexPos;
}

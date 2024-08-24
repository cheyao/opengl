#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexPos;

out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform mat4 model;

uniform float width;
uniform float height;

void main() {
	gl_Position = proj * model * vec4(aPos.x * width, aPos.y * height, aPos.z, 1.0f);
	vTexPos = aTexPos;
}

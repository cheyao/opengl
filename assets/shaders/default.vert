#version 410 core

layout (location = 0) in vec3 aPos;

layout(std140) uniform Matrices {
	mat4 proj;
};

void main() {
	gl_Position = proj * vec4(aPos, 1.0f);
}

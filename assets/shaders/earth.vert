#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vNormal;
out vec3 vFragPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform mat4 model;

void main() {
	vNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
	vFragPos = vec3(model * vec4(aPos, 1.0f));
	gl_Position = proj * view * vec4(vFragPos, 1.0f);
}

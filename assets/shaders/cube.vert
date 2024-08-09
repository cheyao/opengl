#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexPos;
layout (location = 3) in mat4 aMatrix;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexPos;

layout(std140) uniform Matrices {
	mat4 proj;
	mat4 view;
};
uniform mat4 model;
uniform mat4 rotation;

void main() {
	vs_out.normal = aNormal;
	vs_out.fragPos = vec3(rotation * aMatrix * vec4(aPos, 1.0f));
	vs_out.texPos = aTexPos;
	gl_Position = proj * view * vec4(vs_out.fragPos, 1.0f);
}

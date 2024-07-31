#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexPos;

out vec3 normal;
out vec3 fragPos;
out vec2 texPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	normal = mat3(transpose(inverse(model))) * aNormal;

	fragPos = vec3(model * vec4(aPos, 1.0f));
	gl_Position = proj * view * vec4(fragPos, 1.0f);
	texPos = aTexPos;
}

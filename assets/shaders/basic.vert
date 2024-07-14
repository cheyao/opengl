#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexPos;

out vec3 normal;
out vec3 pos;
out vec3 lightPos;
out vec2 texPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 aLightPos;

void main() {
	normal = mat3(transpose(inverse(view * model))) * aNormal;
	pos = vec3(view * model * vec4(aPos, 1.0f));
	lightPos = vec3(view * vec4(aLightPos, 1.0f));
    gl_Position = proj * vec4(pos, 1.0f);
	texPos = aTexPos;
}
